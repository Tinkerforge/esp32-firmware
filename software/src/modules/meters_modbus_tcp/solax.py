specs = [
    {
        'name': 'Solax Hybrid Inverter Grid',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Feedin Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 70,
                'value_type': 'S32LE',
                'scale_factor': -1.0,
            },
            {
                'name': 'Feedin Energy Total [0.01 kWh]',
                'value_id': 'EnergyActiveLSumExport',
                'start_address': 72,
                'value_type': 'S32LE',
                'scale_factor': 0.01,
            },
            {
                'name': 'Consume Energy Total [0.01 kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 74,
                'value_type': 'S32LE',
                'scale_factor': 0.01,
            },
        ],
    },
    {
        'name': 'Solax Hybrid Inverter Battery',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Battery Voltage [0.1 V]',
                'value_id': 'VoltageDC',
                'start_address': 20,
                'value_type': 'S16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery Current [0.1 A]',
                'value_id': 'CurrentDCChaDisDiff',
                'start_address': 21,
                'value_type': 'S16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery Power [W]',
                'value_id': 'PowerDCChaDisDiff',
                'start_address': 22,
                'value_type': 'S16',
            },
            {
                'name': 'Battery Temperature [°C]',
                'value_id': 'Temperature',
                'start_address': 24,
                'value_type': 'S16',
            },
            {
                'name': 'Battery State Of Charge [%]',
                'value_id': 'StateOfCharge',
                'start_address': 28,
                'value_type': 'U16',
            },
            {
                'name': 'Output Energy [0.1 kWh]',
                'value_id': 'EnergyDCDischarge',
                'start_address': 29,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Input Energy [0.1 kWh]',
                'value_id': 'EnergyDCCharge',
                'start_address': 33,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
        ],
    },
]
