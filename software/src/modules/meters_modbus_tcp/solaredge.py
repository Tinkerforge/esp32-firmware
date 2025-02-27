specs = [
    {
        'name': 'Solaredge Inverter Battery',
        'register_type': 'HoldingRegister',
        'start_address_offset': 0,
        'values': [
            {
                'name': 'Battery 1 Average Temperature [°C]',
                'value_id': 'Temperature',
                'start_address': 0xE16C,
                'value_type': 'F32LE',
            },
            {
                'name': 'Battery 1 Instantaneous Voltage [V]',
                'value_id': 'VoltageDC',
                'start_address': 0xE170,
                'value_type': 'F32LE',
            },
            {
                'name': 'Battery 1 Instantaneous Current [A]',
                'value_id': 'CurrentDCChaDisDiff',
                'start_address': 0xE172,
                'value_type': 'F32LE',
            },
            {
                'name': 'Battery 1 Instantaneous Power [W]',
                'value_id': 'PowerDCChaDisDiff',
                'start_address': 0xE174,
                'value_type': 'F32LE',
            },
            {
                'name': 'Battery 1 Lifetime Export Energy Counter [0.1 kWh]',
                'value_id': 'EnergyDCDischarge',
                'start_address': 0xE176,
                'value_type': 'F32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery 1 Lifetime Import Energy Counter [0.1 kWh]',
                'value_id': 'EnergyDCCharge',
                'start_address': 0xE17A,
                'value_type': 'F32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery 1 State of Energy [%]',
                'value_id': 'StateOfCharge',
                'start_address': 0xE184,
                'value_type': 'F32LE',
            },
        ],
    },
]
