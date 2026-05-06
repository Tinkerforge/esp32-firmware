display_names = [
    ('Eltako DSZ16DZE', {
        'en': 'Eltako DSZ16D[ZE]',
        'de': 'Eltako DSZ16D[ZE]',
    }),
]

table_prototypes = [
    ('Eltako DSZ16DZE', ['device_address']),
]

default_device_addresses = [
    ('Eltako DSZ16DZE', 1),
]

specs = [
    {
        'name': 'Eltako DSZ16DZE',
        'default_location': 'Unknown',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Spannung L1 zu N [0.01 V]',
                'value_id': 'VoltageL1N',
                'start_address': 0,
                'value_type': 'S32BE',
                'scale_factor': 0.01,
            },
            {
                'name': 'Spannung L2 zu N [0.01 V]',
                'value_id': 'VoltageL2N',
                'start_address': 2,
                'value_type': 'S32BE',
                'scale_factor': 0.01,
            },
            {
                'name': 'Spannung L3 zu N [0.01 V]',
                'value_id': 'VoltageL3N',
                'start_address': 4,
                'value_type': 'S32BE',
                'scale_factor': 0.01,
            },
            {
                'name': 'Strom L1 [0.01 A]',
                'value_id': 'CurrentL1ImExSum',
                'start_address': 6,
                'value_type': 'S32BE',
                'scale_factor': 0.01,
            },
            {
                'name': 'Strom L2 [0.01 A]',
                'value_id': 'CurrentL2ImExSum',
                'start_address': 8,
                'value_type': 'S32BE',
                'scale_factor': 0.01,
            },
            {
                'name': 'Strom L3 [0.01 A]',
                'value_id': 'CurrentL3ImExSum',
                'start_address': 10,
                'value_type': 'S32BE',
                'scale_factor': 0.01,
            },
            {
                'name': 'Wirkleistung L1 [W]',
                'value_id': 'PowerActiveL1ImExDiff',
                'start_address': 12,
                'value_type': 'S32BE',
            },
            {
                'name': 'Wirkleistung L2 [W]',
                'value_id': 'PowerActiveL2ImExDiff',
                'start_address': 14,
                'value_type': 'S32BE',
            },
            {
                'name': 'Wirkleistung L3 [W]',
                'value_id': 'PowerActiveL3ImExDiff',
                'start_address': 16,
                'value_type': 'S32BE',
            },
            {
                'name': 'Scheinleistung L1 [VA]',
                'value_id': 'PowerApparentL1ImExSum',
                'start_address': 18,
                'value_type': 'S32BE',
            },
            {
                'name': 'Scheinleistung L2 [VA]',
                'value_id': 'PowerApparentL2ImExSum',
                'start_address': 20,
                'value_type': 'S32BE',
            },
            {
                'name': 'Scheinleistung L3 [VA]',
                'value_id': 'PowerApparentL3ImExSum',
                'start_address': 22,
                'value_type': 'S32BE',
            },
            {
                'name': 'Blindleistung L1 [var]',
                'value_id': 'PowerReactiveL1IndCapDiff',
                'start_address': 24,
                'value_type': 'S32BE',
            },
            {
                'name': 'Blindleistung L2 [var]',
                'value_id': 'PowerReactiveL2IndCapDiff',
                'start_address': 26,
                'value_type': 'S32BE',
            },
            {
                'name': 'Blindleistung L3 [var]',
                'value_id': 'PowerReactiveL3IndCapDiff',
                'start_address': 28,
                'value_type': 'S32BE',
            },
            {
                'name': 'Power Faktor L1 [0.001]',
                'value_id': 'PowerFactorL1Directional',
                'start_address': 30,
                'value_type': 'S32BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Power Faktor L2 [0.001]',
                'value_id': 'PowerFactorL2Directional',
                'start_address': 32,
                'value_type': 'S32BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Power Faktor L3 [0.001]',
                'value_id': 'PowerFactorL3Directional',
                'start_address': 34,
                'value_type': 'S32BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Phasenwinkel L1 [0.001 °]',
                'value_id': 'PhaseAngleL1',
                'start_address': 36,
                'value_type': 'S32BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Phasenwinkel L2 [0.001 °]',
                'value_id': 'PhaseAngleL2',
                'start_address': 38,
                'value_type': 'S32BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Phasenwinkel L3 [0.001 °]',
                'value_id': 'PhaseAngleL3',
                'start_address': 40,
                'value_type': 'S32BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Gesamte Wirkleistung [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 52,
                'value_type': 'S32BE',
            },
            {
                'name': 'Gesamte Scheinleistung [VA]',
                'value_id': 'PowerApparentLSumImExSum',
                'start_address': 54,
                'value_type': 'S32BE',
            },
            {
                'name': 'Gesamte Blindleistung [var]',
                'value_id': 'PowerReactiveLSumIndCapDiff',
                'start_address': 56,
                'value_type': 'S32BE',
            },
            {
                'name': 'Gesamter Power Faktor [0.001]',
                'value_id': 'PowerFactorLSumDirectional',
                'start_address': 62,
                'value_type': 'S32BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Gesamter Phasenwinkel [0.001 °]',
                'value_id': 'PhaseAngleLSum',
                'start_address': 64,
                'value_type': 'S32BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Frequenz [0.01 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 70,
                'value_type': 'S32BE',
                'scale_factor': 0.01,
            },
            {
                'name': 'Gesamte importierte Wirkenergie [0.01 kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 72,
                'value_type': 'S32BE',
                'scale_factor': 0.01,
            },
            {
                'name': 'Gesamte exportierte Wirkenergie [0.01 kWh]',
                'value_id': 'EnergyActiveLSumExport',
                'start_address': 74,
                'value_type': 'S32BE',
                'scale_factor': 0.01,
            },
        ],
    },
]
