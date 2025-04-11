specs = [
    {
        'name': 'SAX Power Home Basic Mode Grid',  # Port: 3600 (and 502?), Unit ID: 64
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Leistung des Smart Meters [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 48,
                'value_type': 'U16',
                'offset': -16384.0,
            },
        ],
    },
    {
        'name': 'SAX Power Home Basic Mode Battery',  # Port: 3600 (and 502?), Unit ID: 64
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'SOC des Speichers [%]',
                'value_id': 'StateOfCharge',
                'start_address': 46,
                'value_type': 'U16',
            },
            {
                'name': 'Leistung P des Speichers [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 47,
                'value_type': 'U16',
                'scale_factor': -1.0,
                'offset': -16384.0,
            },
        ],
    },
    {
        'name': 'SAX Power Home Extended Mode Grid',  # Port: 502, Unit ID: 40
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Energie Eingespeist [10 kWh]',
                'value_id': 'EnergyActiveLSumExport',
                'start_address': 95,
                'value_type': 'U16',
                'scale_factor': 10.0,
            },
            {
                'name': 'Energie Bezogen [10 kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 96,
                'value_type': 'U16',
                'scale_factor': 10.0,
            },
            {
                'name': 'Strom L1 [0.01 A]',
                'value_id': 'CurrentL1ImExDiff',
                'start_address': 99,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Strom L2 [0.01 A]',
                'value_id': 'CurrentL2ImExDiff',
                'start_address': 100,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Strom L3 [0.01 A]',
                'value_id': 'CurrentL3ImExDiff',
                'start_address': 101,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Wirkleistung L1 [10 W]',
                'value_id': 'PowerActiveL1ImExDiff',
                'start_address': 102,
                'value_type': 'S16',
                'scale_factor': 10.0,  # FIXME: sign?
            },
            {
                'name': 'Wirkleistung L2 [10 W]',
                'value_id': 'PowerActiveL2ImExDiff',
                'start_address': 103,
                'value_type': 'S16',
                'scale_factor': 10.0,  # FIXME: sign?
            },
            {
                'name': 'Wirkleistung L3 [10 W]',
                'value_id': 'PowerActiveL3ImExDiff',
                'start_address': 104,
                'value_type': 'S16',
                'scale_factor': 10.0,  # FIXME: sign?
            },
            {
                'name': 'Spannung L1 [0.1 V]',
                'value_id': 'VoltageL1N',
                'start_address': 106,
                'value_type': 'S16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Spannung L2 [0.1 V]',
                'value_id': 'VoltageL2N',
                'start_address': 107,
                'value_type': 'S16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Spannung L3 [0.1 V]',
                'value_id': 'VoltageL3N',
                'start_address': 108,
                'value_type': 'S16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Summenleistung Wirk [10 W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 109,
                'value_type': 'S16',
                'scale_factor': 10.0,  # FIXME: sign?
            },
        ],
    },
    {
        'name': 'SAX Power Home Extended Mode Battery',  # Port: 502, Unit ID: 40
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Summe Phasenstrom [0.01 A]',
                'value_id': 'CurrentLSumImExSum',
                'start_address': 72,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
            {
                'name': 'Strom L1 [0.01 A]',
                'value_id': 'CurrentL1ImExSum',
                'start_address': 73,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
            {
                'name': 'Strom L2 [0.01 A]',
                'value_id': 'CurrentL2ImExSum',
                'start_address': 74,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
            {
                'name': 'Strom L3 [0.01 A]',
                'value_id': 'CurrentL3ImExSum',
                'start_address': 75,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
            {
                'name': 'Spannung L1 [0.1 V]',
                'value_id': 'VoltageL1N',
                'start_address': 80,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Spannung L2 [0.1 V]',
                'value_id': 'VoltageL2N',
                'start_address': 81,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Spannung L3 [0.1 V]',
                'value_id': 'VoltageL3N',
                'start_address': 82,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Summenleistung AC Wirk [10 W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 84,
                'value_type': 'S16',
                'scale_factor': 10.0,  # FIXME: sign?
            },
            {
                'name': 'Netzfrequenz [0.1 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 86,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Summenleistung AC Schein [10 VA]',
                'value_id': 'PowerApparentLSumImExDiff',
                'start_address': 88,
                'value_type': 'S16',
                'scale_factor': 10.0,  # FIXME: sign?
            },
            {
                'name': 'Summenleistung AC Blind [10 var]',
                'value_id': 'PowerReactiveLSumIndCapDiff',
                'start_address': 90,
                'value_type': 'S16',
                'scale_factor': 10.0,  # FIXME: sign?
            },
            {
                'name': 'Leistungsfaktor [0.1]',
                'value_id': 'PowerFactorLSumDirectional',  # FIXME: directional?
                'start_address': 92,
                'value_type': 'S16',
                'scale_factor': 0.1,  # FIXME: sign?
            },
            {
                'name': 'SOC [%]',  # FIXME: not available?
                'value_id': 'StateOfCharge',
                'start_address': 113,
                'value_type': 'S16',
            },
            {
                'name': 'Temperatur [°C]',  # FIXME: not available?
                'value_id': 'Temperature',
                'start_address': 116,
                'value_type': 'S16',
            },
        ],
    },
]
