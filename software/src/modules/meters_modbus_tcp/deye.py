specs = [
    {
        'name': 'Deye Hybrid Inverter',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Rated Power [0.1 W]',
                'value_id': 'VALUE_ID_DEBUG',
                'start_address': 20,
                'value_type': 'U32LE',
            },
            {
                'name': 'Total Active Power Generation [0.1 kWh]',
                'value_id': 'EnergyActiveLSumExport',
                'start_address': 504,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Total Reactive Power Generation [0.1 kWh]',
                'value_id': 'EnergyReactiveLSumIndCapSum',  # FIXME: sum vs diff?
                'start_address': 506,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Total PV Power [0.1 kWh]',
                'value_id': 'EnergyDCExport',
                'start_address': 534,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Transformer Temperature [0.1 °C]',
                'value_id': 'TemperatureTransformer',
                'start_address': 540,
                'value_type': 'U16',
                'offset': -1000.0,
                'scale_factor': 0.1,
            },
            {
                'name': 'Heat Sink Temperature [0.1 °C]',
                'value_id': 'TemperatureHeatSink',
                'start_address': 541,
                'value_type': 'U16',
                'offset': -1000.0,
                'scale_factor': 0.1,
            },
            {
                'name': 'Output Phase Voltage A [0.1 V]',
                'value_id': 'VoltageL1N',
                'start_address': 627,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Output Phase Voltage B [0.1 V]',
                'value_id': 'VoltageL2N',
                'start_address': 628,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Output Phase Voltage C [0.1 V]',
                'value_id': 'VoltageL3N',
                'start_address': 629,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Output Phase Current A [0.01 A]',
                'value_id': 'CurrentL1ImExDiff',  # FIXME: direction?
                'start_address': 630,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Output Phase Current B [0.01 A]',
                'value_id': 'CurrentL2ImExDiff',  # FIXME: direction?
                'start_address': 631,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Output Phase Current C [0.01 A]',
                'value_id': 'CurrentL3ImExDiff',  # FIXME: direction?
                'start_address': 632,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Output Phase Power A [W]',
                'value_id': 'PowerActiveL1ImExDiff',  # FIXME: direction?
                'start_address': 633,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Output Phase Power B [W]',
                'value_id': 'PowerActiveL2ImExDiff',  # FIXME: direction?
                'start_address': 634,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Output Phase Power C [W]',
                'value_id': 'PowerActiveL3ImExDiff',  # FIXME: direction?
                'start_address': 635,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Output Total Active Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',  # FIXME: direction?
                'start_address': 636,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Output Total Apparent Power [VA]',
                'value_id': 'PowerApparentLSumImExDiff',  # FIXME: direction?
                'start_address': 637,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Frequency [0.01 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 638,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
        ],
    },
    {
        'name': 'Deye Hybrid Inverter Grid',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Total Grid Buy [0.1 kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 522,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Total Grid Sell [0.1 kWh]',
                'value_id': 'EnergyActiveLSumExport',
                'start_address': 524,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Grid Side Phase Voltage A [0.1 V]',
                'value_id': 'VoltageL1N',
                'start_address': 598,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Grid Side Phase Voltage B [0.1 V]',
                'value_id': 'VoltageL2N',
                'start_address': 599,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Grid Side Phase Voltage C [0.1 V]',
                'value_id': 'VoltageL3N',
                'start_address': 600,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Grid Side Line Voltage A-B [0.1 V]',
                'value_id': 'VALUE_ID_DEBUG',  # FIXME: there seems to be no way to detect 3P4L vs 3P3L, ignore this voltage for now
                'start_address': 601,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Grid Side Line Voltage B-C [0.1 V]',
                'value_id': 'VALUE_ID_DEBUG',  # FIXME: there seems to be no way to detect 3P4L vs 3P3L, ignore this voltage for now
                'start_address': 602,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Grid Side Line Voltage C-A [0.1 V]',
                'value_id': 'VALUE_ID_DEBUG',  # FIXME: there seems to be no way to detect 3P4L vs 3P3L, ignore this voltage for now
                'start_address': 603,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Grid Side Internal Power A [W]',
                'value_id': 'VALUE_ID_DEBUG',
                'start_address': 604,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side Internal Power B [W]',
                'value_id': 'VALUE_ID_DEBUG',
                'start_address': 605,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side Internal Power C [W]',
                'value_id': 'VALUE_ID_DEBUG',
                'start_address': 606,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side Internal Total Active Power [W]',
                'value_id': 'VALUE_ID_DEBUG',
                'start_address': 607,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side Internal Total Apparent Power [VA]',
                'value_id': 'VALUE_ID_DEBUG',
                'start_address': 608,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side Frequency [0.01 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 609,
                'value_type': 'U16',
                'scale_factor': 0.01,  # FIXME: undocumented
            },
            {
                'name': 'Grid Side Internal Current A [0.01 A]',
                'value_id': 'VALUE_ID_DEBUG',
                'start_address': 610,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Grid Side Internal Current B [0.01 A]',
                'value_id': 'VALUE_ID_DEBUG',
                'start_address': 611,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Grid Side Internal Current C [0.01 A]',
                'value_id': 'VALUE_ID_DEBUG',
                'start_address': 612,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Grid Side External Current A [0.01 A]',
                'value_id': 'CurrentL1ImExDiff',  # FIXME: direction?
                'start_address': 613,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Grid Side External Current B [0.01 A]',
                'value_id': 'CurrentL2ImExDiff',  # FIXME: direction?
                'start_address': 614,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Grid Side External Current C [0.01 A]',
                'value_id': 'CurrentL3ImExDiff',  # FIXME: direction?
                'start_address': 615,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Grid Side External Power A [W]',
                'value_id': 'PowerActiveL1ImExDiff',  # FIXME: direction?
                'start_address': 616,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side External Power B [W]',
                'value_id': 'PowerActiveL2ImExDiff',  # FIXME: direction?
                'start_address': 617,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side External Power C [W]',
                'value_id': 'PowerActiveL3ImExDiff',  # FIXME: direction?
                'start_address': 618,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side External Total Active Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',  # FIXME: direction?
                'start_address': 619,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side External Total Apparent Power [VA]',
                'value_id': 'PowerApparentLSumImExDiff',  # FIXME: direction?
                'start_address': 620,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side Power Factor',
                'value_id': 'PowerFactorLSum',
                'start_address': 621,
                'value_type': 'U16',
                'scale_factor': 0.001,
            },
            {
                'name': 'Grid Side Power A [W]',
                'value_id': 'VALUE_ID_DEBUG',
                'start_address': 622,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side Power B [W]',
                'value_id': 'VALUE_ID_DEBUG',
                'start_address': 623,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side Power C [W]',
                'value_id': 'VALUE_ID_DEBUG',
                'start_address': 624,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Grid Side Total Active Power [W]',
                'value_id': 'VALUE_ID_DEBUG',
                'start_address': 625,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
        ],
    },
    {
        'name': 'Deye Hybrid Inverter Device Type',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Device Type',
                'value_id': 'VALUE_ID_META',
                'start_address': 0,
                'value_type': 'U16',
            },
        ],
    },
    {
        'name': 'Deye Hybrid Inverter {variant} Battery',
        'variants': ['Low Voltage', 'High Voltage'],
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Total Battery Charge [0.1 kWh]',
                'value_id': 'EnergyDCCharge',
                'start_address': 516,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Total Battery Discharge [0.1 kWh]',
                'value_id': 'EnergyDCDischarge',
                'start_address': 518,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery Temperature [0.1 °C]',
                'value_id': 'Temperature',
                'start_address': 586,
                'value_type': 'U16',
                'offset': -1000.0,
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery Voltage [0.01 V]',
                'value_id': 'VoltageDC',
                'start_address': 587,
                'value_type': 'U16',
                'scale_factor': 0.01,
                'variant': 'Low Voltage',
            },
            {
                'name': 'Battery Voltage [0.1 V]',
                'value_id': 'VoltageDC',
                'start_address': 587,
                'value_type': 'U16',
                'scale_factor': 0.1,
                'variant': 'High Voltage',
            },
            {
                'name': 'Battery Capacity [%]',
                'value_id': 'StateOfCharge',
                'start_address': 588,
                'value_type': 'U16',
            },
            {
                'name': 'Battery Power [W]',
                'value_id': 'PowerDCChaDisDiff',
                'start_address': 590,
                'value_type': 'S16',
                'scale_factor': -1.0,
                'variant': 'Low Voltage',
            },
            {
                'name': 'Battery Power [10 W]',
                'value_id': 'PowerDCChaDisDiff',
                'start_address': 590,
                'value_type': 'S16',
                'scale_factor': -10.0,
                'variant': 'High Voltage',
            },
            {
                'name': 'Battery Current [0.01 A]',
                'value_id': 'CurrentDCChaDisDiff',
                'start_address': 591,
                'value_type': 'S16',
                'scale_factor': -0.01,
            },
        ],
    },
    {
        'name': 'Deye Hybrid Inverter Load',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Total Load Power [0.1 kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 527,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Load Phase Voltage A [0.1 V]',
                'value_id': 'VoltageL1N',
                'start_address': 644,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Load Phase Voltage B [0.1 V]',
                'value_id': 'VoltageL2N',
                'start_address': 645,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Load Phase Voltage C [0.1 V]',
                'value_id': 'VoltageL3N',
                'start_address': 646,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Load Phase Power A [W]',
                'value_id': 'PowerActiveL1ImExDiff',  # FIXME: direction?
                'start_address': 650,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Load Phase Power B [W]',
                'value_id': 'PowerActiveL2ImExDiff',  # FIXME: direction?
                'start_address': 651,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Load Phase Power C [W]',
                'value_id': 'PowerActiveL3ImExDiff',  # FIXME: direction?
                'start_address': 652,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Load Total Active Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',  # FIXME: direction?
                'start_address': 653,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Load Total Apparent Power [VA]',
                'value_id': 'PowerApparentLSumImExDiff',  # FIXME: direction?
                'start_address': 654,
                'value_type': 'S16',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Load Frequency [0.01 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 655,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
        ],
    },
]
