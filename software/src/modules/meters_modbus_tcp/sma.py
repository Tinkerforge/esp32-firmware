specs = [
    {
        'name': 'SMA Hybrid Inverter Battery',  # FIXME: handle NAN markers
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Bat.Amp [0.001 A]',
                'value_id': 'CurrentDCChaDisDiff',
                'start_address': 30843,
                'value_type': 'S32BE',
                'scale_factor': -0.001,
            },
            {
                'name': 'Bat.ChaStt [%]',
                'value_id': 'StateOfCharge',
                'start_address': 30845,
                'value_type': 'U32BE',
            },
            {
                'name': 'Bat.TmpVal [0.1 °C]',
                'value_id': 'Temperature',
                'start_address': 30849,
                'value_type': 'S32BE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Bat.Vol [0.01 V]',
                'value_id': 'VoltageDC',
                'start_address': 30851,
                'value_type': 'U32BE',
                'scale_factor': 0.01,
            },
            {
                'name': 'BatChrg.CurBatCha [W]',
                'value_id': 'VALUE_ID_META',
                'start_address': 31393,
                'value_type': 'U32BE',
            },
            {
                'name': 'BatDsch.CurBatDsch [W]',
                'value_id': 'VALUE_ID_META',
                'start_address': 31395,
                'value_type': 'U32BE',
            },
            {
                'name': 'Battery Power [W]',
                'value_id': 'PowerDCChaDisDiff',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
            {
                'name': 'BatChrg.BatChrg [0.001 kWh]',
                'value_id': 'EnergyDCCharge',
                'start_address': 31397,
                'value_type': 'U64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'BatDsch.BatDsch [0.001 kWh]',
                'value_id': 'EnergyDCDischarge',
                'start_address': 31401,
                'value_type': 'U64BE',
                'scale_factor': 0.001,
            },
        ],
    },
]
