specs = [
    {
        'name': 'E3DC Grid',
        'register_type': 'HoldingRegister',
        'start_address_offset': 1,
        'values': [
            {
                'name': 'Leistung am Netzübergabepunkt [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 74,
                'value_type': 'S32LE',
            },
        ],
    },
    {
        'name': 'E3DC Battery',
        'register_type': 'HoldingRegister',
        'start_address_offset': 1,
        'values': [
            {
                'name': 'Batterie-Leistung [W]',
                'value_id': 'PowerDCChaDisDiff',
                'start_address': 70,
                'value_type': 'S32LE',
            },
            {
                'name': 'Batterie-SOC [%]',
                'value_id': 'StateOfCharge',
                'start_address': 83,
                'value_type': 'U16',
            },
        ],
    },
    {
        'name': 'E3DC Load',
        'register_type': 'HoldingRegister',
        'start_address_offset': 1,
        'values': [
            {
                'name': 'Hausverbrauchs-Leistung [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 72,
                'value_type': 'S32LE',
            },
        ],
    },
    {
        'name': 'E3DC PV',
        'register_type': 'HoldingRegister',
        'start_address_offset': 1,
        'values': [
            {
                'name': 'DC String 1 Voltage [V]',
                'value_id': 'VoltagePV1',
                'start_address': 96,
                'value_type': 'U16',
            },
            {
                'name': 'DC String 2 Voltage [V]',
                'value_id': 'VoltagePV2',
                'start_address': 97,
                'value_type': 'U16',
            },
            {
                'name': 'DC String 3 Voltage [V]',
                'value_id': 'VoltagePV3',
                'start_address': 98,
                'value_type': 'U16',
            },
            {
                'name': 'DC String 1 Current [0.01 A]',
                'value_id': 'CurrentPV1Export',
                'start_address': 99,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
            {
                'name': 'DC String 2 Current [0.01 A]',
                'value_id': 'CurrentPV2Export',
                'start_address': 100,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
            {
                'name': 'DC String 3 Current [0.01 A]',
                'value_id': 'CurrentPV3Export',
                'start_address': 101,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
            {
                'name': 'DC String 1 Power [W]',
                'value_id': 'PowerPV1Export',
                'start_address': 102,
                'value_type': 'U16',
            },
            {
                'name': 'DC String 2 Power [W]',
                'value_id': 'PowerPV2Export',
                'start_address': 103,
                'value_type': 'U16',
            },
            {
                'name': 'DC String 3 Power [W]',
                'value_id': 'PowerPV3Export',
                'start_address': 104,
                'value_type': 'U16',
            },
            {
                'name': 'Photovoltaik-Leistung [W] | PV Power',
                'value_id': 'PowerPVSumExport',
                'start_address': 68,
                'value_type': 'S32LE',
            },
            {
                'name': 'Photovoltaik-Leistung Signed [W]',
                'value_id': 'PowerPVSumImExDiff',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
        ],
    },
    {
        'name': 'E3DC Additional Generation',
        'register_type': 'HoldingRegister',
        'start_address_offset': 1,
        'values': [
            {
                'name': 'Leistung aller zusätzlichen Einspeiser [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 76,
                'value_type': 'S32LE',
            },
        ],
    },
]
