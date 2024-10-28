specs = [
    {
        'name': 'Siemens PAC2200',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Spannung U L1-N [V]',
                'value_id': 'VoltageL1N',
                'start_address': 1,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung U L2-N [V]',
                'value_id': 'VoltageL2N',
                'start_address': 3,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung U L3-N [V]',
                'value_id': 'VoltageL3N',
                'start_address': 5,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung U L1-L2 [V]',
                'value_id': 'VoltageL1L2',
                'start_address': 7,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung U L2-L3 [V]',
                'value_id': 'VoltageL2L3',
                'start_address': 9,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung U L3-L1 [V]',
                'value_id': 'VoltageL3L1',
                'start_address': 11,
                'value_type': 'F32BE',
            },
            {
                'name': 'Strom L1 [A]',
                'value_id': 'CurrentL1ImExDiff',  # FIXME: direction?
                'start_address': 13,
                'value_type': 'F32BE',
            },
            {
                'name': 'Strom L2 [A]',
                'value_id': 'CurrentL2ImExDiff',  # FIXME: direction?
                'start_address': 15,
                'value_type': 'F32BE',
            },
            {
                'name': 'Strom L3 [A]',
                'value_id': 'CurrentL3ImExDiff',  # FIXME: direction?
                'start_address': 17,
                'value_type': 'F32BE',
            },
            {
                'name': 'Scheinleistung L1 [VA]',
                'value_id': 'PowerApparentL1ImExDiff',  # FIXME: direction?
                'start_address': 19,
                'value_type': 'F32BE',
            },
            {
                'name': 'Scheinleistung L2 [VA]',
                'value_id': 'PowerApparentL2ImExDiff',  # FIXME: direction?
                'start_address': 21,
                'value_type': 'F32BE',
            },
            {
                'name': 'Scheinleistung L3 [VA]',
                'value_id': 'PowerApparentL3ImExDiff',  # FIXME: direction?
                'start_address': 23,
                'value_type': 'F32BE',
            },
            {
                'name': 'Wirkleistung L1 [W]',
                'value_id': 'PowerActiveL1ImExDiff',  # FIXME: direction?
                'start_address': 25,
                'value_type': 'F32BE',
            },
            {
                'name': 'Wirkleistung L2 [W]',
                'value_id': 'PowerActiveL2ImExDiff',  # FIXME: direction?
                'start_address': 27,
                'value_type': 'F32BE',
            },
            {
                'name': 'Wirkleistung L3 [W]',
                'value_id': 'PowerActiveL3ImExDiff',  # FIXME: direction?
                'start_address': 29,
                'value_type': 'F32BE',
            },
            {
                'name': 'Blindleistung L1 [var]',
                'value_id': 'PowerReactiveL1IndCapDiff',  # FIXME: direction?
                'start_address': 31,
                'value_type': 'F32BE',
            },
            {
                'name': 'Blindleistung L2 [var]',
                'value_id': 'PowerReactiveL2IndCapDiff',  # FIXME: direction?
                'start_address': 33,
                'value_type': 'F32BE',
            },
            {
                'name': 'Blindleistung L3 [var]',
                'value_id': 'PowerReactiveL3IndCapDiff',  # FIXME: direction?
                'start_address': 35,
                'value_type': 'F32BE',
            },
            {
                'name': 'Leistungsfaktor L1',
                'value_id': 'PowerFactorL1',
                'start_address': 37,
                'value_type': 'F32BE',
            },
            {
                'name': 'Leistungsfaktor L2',
                'value_id': 'PowerFactorL2',
                'start_address': 39,
                'value_type': 'F32BE',
            },
            {
                'name': 'Leistungsfaktor L3',
                'value_id': 'PowerFactorL3',
                'start_address': 41,
                'value_type': 'F32BE',
            },
            {
                'name': 'Frequenz [Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 55,
                'value_type': 'F32BE',
            },
            {
                'name': 'Mittelwert Spannung U L-N [V]',
                'value_id': 'VoltageLNAvg',
                'start_address': 57,
                'value_type': 'F32BE',
            },
            {
                'name': 'Mittelwert Spannung U L-L [V]',
                'value_id': 'VoltageLLAvg',
                'start_address': 59,
                'value_type': 'F32BE',
            },
            {
                'name': 'Mittelwert Strom [A]',
                'value_id': 'CurrentLAvgImExDiff',  # FIXME: direction?
                'start_address': 61,
                'value_type': 'F32BE',
            },
            {
                'name': 'Summe der Scheinleistung [VA]',
                'value_id': 'PowerApparentLSumImExDiff',  # FIXME: direction?
                'start_address': 63,
                'value_type': 'F32BE',
            },
            {
                'name': 'Summe der Wirkleistung [W]',
                'value_id': 'PowerActiveLSumImExDiff',  # FIXME: direction?
                'start_address': 65,
                'value_type': 'F32BE',
            },
            {
                'name': 'Summe der Blindleistung [var]',
                'value_id': 'PowerReactiveLSumIndCapDiff',  # FIXME: direction?
                'start_address': 67,
                'value_type': 'F32BE',
            },
            {
                'name': 'Gesamt-Leistungsfaktor',
                'value_id': 'PowerFactorLSum',
                'start_address': 69,
                'value_type': 'F32BE',
            },
            {
                'name': 'Neutralleiterstrom [A]',
                'value_id': 'CurrentNImExDiff',  # FIXME: direction?
                'start_address': 71,
                'value_type': 'F32BE',
            },
            {
                'name': 'Wirkarbeit Bezug Tarif 1 [Wh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 801,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Wirkarbeit Abgabe Tarif 1 [Wh]',
                'value_id': 'EnergyActiveLSumExport',
                'start_address': 809,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Blindarbeit Bezug Tarif 1 [varh]',
                'value_id': 'EnergyReactiveLSumInductive',  # FIXME: direction?
                'start_address': 817,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Blindarbeit Abgabe Tarif 1 [varh]',
                'value_id': 'EnergyReactiveLSumCapacitive',  # FIXME: direction?
                'start_address': 825,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Scheinarbeit Tarif 1 [VAh]',
                'value_id': 'EnergyApparentLSumImExSum',  # FIXME: sum?
                'start_address': 833,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L1 Wirkarbeit Bezug Tarif 1 [Wh]',
                'value_id': 'EnergyActiveL1Import',
                'start_address': 841,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L1 Wirkarbeit Abgabe Tarif 1 [Wh]',
                'value_id': 'EnergyActiveL1Export',
                'start_address': 849,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L1 Blindarbeit Bezug Tarif 1 [varh]',
                'value_id': 'EnergyReactiveL1Inductive',  # FIXME: direction?
                'start_address': 857,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L1 Blindarbeit Abgabe Tarif 1 [varh]',
                'value_id': 'EnergyReactiveL1Capacitive',  # FIXME: direction?
                'start_address': 865,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L1 Scheinarbeit Tarif 1 [VAh]',
                'value_id': 'EnergyApparentL1ImExSum',  # FIXME: sum?
                'start_address': 873,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L2 Wirkarbeit Bezug Tarif 1 [Wh]',
                'value_id': 'EnergyActiveL2Import',
                'start_address': 881,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L2 Wirkarbeit Abgabe Tarif 1 [Wh]',
                'value_id': 'EnergyActiveL2Export',
                'start_address': 889,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L2 Blindarbeit Bezug Tarif 1 [varh]',  # FIXME: direction?
                'value_id': 'EnergyReactiveL2Inductive',
                'start_address': 897,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L2 Blindarbeit Abgabe Tarif 1 [varh]',  # FIXME: direction?
                'value_id': 'EnergyReactiveL2Capacitive',
                'start_address': 905,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L2 Scheinarbeit Tarif 1 [VAh]',
                'value_id': 'EnergyApparentL2ImExSum',  # FIXME: sum?
                'start_address': 913,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L3 Wirkarbeit Bezug Tarif 1 [Wh]',
                'value_id': 'EnergyActiveL3Import',
                'start_address': 921,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L3 Wirkarbeit Abgabe Tarif 1 [Wh]',
                'value_id': 'EnergyActiveL3Export',
                'start_address': 929,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L3 Blindarbeit Bezug Tarif 1 [varh]',
                'value_id': 'EnergyReactiveL3Inductive',  # FIXME: direction?
                'start_address': 937,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L3 Blindarbeit Abgabe Tarif 1 [varh]',
                'value_id': 'EnergyReactiveL3Capacitive',  # FIXME: direction?
                'start_address': 945,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L3 Scheinarbeit Tarif 1 [VAh]',
                'value_id': 'EnergyApparentL3ImExSum',  # FIXME: sum?
                'start_address': 953,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
        ],
    },
    {
        'name': 'Siemens PAC3120 and PAC3220',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Spannung U L1-N [V]',
                'value_id': 'VoltageL1N',
                'start_address': 1,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung U L2-N [V]',
                'value_id': 'VoltageL2N',
                'start_address': 3,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung U L3-N [V]',
                'value_id': 'VoltageL3N',
                'start_address': 5,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung U L1-L2 [V]',
                'value_id': 'VoltageL1L2',
                'start_address': 7,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung U L2-L3 [V]',
                'value_id': 'VoltageL2L3',
                'start_address': 9,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung U L3-L1 [V]',
                'value_id': 'VoltageL3L1',
                'start_address': 11,
                'value_type': 'F32BE',
            },
            {
                'name': 'Strom L1 [A]',
                'value_id': 'CurrentL1ImExDiff',  # FIXME: direction?
                'start_address': 13,
                'value_type': 'F32BE',
            },
            {
                'name': 'Strom L2 [A]',
                'value_id': 'CurrentL2ImExDiff',  # FIXME: direction?
                'start_address': 15,
                'value_type': 'F32BE',
            },
            {
                'name': 'Strom L3 [A]',
                'value_id': 'CurrentL3ImExDiff',  # FIXME: direction?
                'start_address': 17,
                'value_type': 'F32BE',
            },
            {
                'name': 'Scheinleistung L1 [VA]',
                'value_id': 'PowerApparentL1ImExDiff',  # FIXME: direction?
                'start_address': 19,
                'value_type': 'F32BE',
            },
            {
                'name': 'Scheinleistung L2 [VA]',
                'value_id': 'PowerApparentL2ImExDiff',  # FIXME: direction?
                'start_address': 21,
                'value_type': 'F32BE',
            },
            {
                'name': 'Scheinleistung L3 [VA]',
                'value_id': 'PowerApparentL3ImExDiff',  # FIXME: direction?
                'start_address': 23,
                'value_type': 'F32BE',
            },
            {
                'name': 'Wirkleistung L1 [W]',
                'value_id': 'PowerActiveL1ImExDiff',  # FIXME: direction?
                'start_address': 25,
                'value_type': 'F32BE',
            },
            {
                'name': 'Wirkleistung L2 [W]',
                'value_id': 'PowerActiveL2ImExDiff',  # FIXME: direction?
                'start_address': 27,
                'value_type': 'F32BE',
            },
            {
                'name': 'Wirkleistung L3 [W]',
                'value_id': 'PowerActiveL3ImExDiff',  # FIXME: direction?
                'start_address': 29,
                'value_type': 'F32BE',
            },
            {
                'name': 'Blindleistung L1 [var]',
                'value_id': 'PowerReactiveL1IndCapDiff',  # FIXME: direction?
                'start_address': 31,
                'value_type': 'F32BE',
            },
            {
                'name': 'Blindleistung L2 [var]',
                'value_id': 'PowerReactiveL2IndCapDiff',  # FIXME: direction?
                'start_address': 33,
                'value_type': 'F32BE',
            },
            {
                'name': 'Blindleistung L3 [var]',
                'value_id': 'PowerReactiveL3IndCapDiff',  # FIXME: direction?
                'start_address': 35,
                'value_type': 'F32BE',
            },
            {
                'name': 'Leistungsfaktor L1',
                'value_id': 'PowerFactorL1',
                'start_address': 37,
                'value_type': 'F32BE',
            },
            {
                'name': 'Leistungsfaktor L2',
                'value_id': 'PowerFactorL2',
                'start_address': 39,
                'value_type': 'F32BE',
            },
            {
                'name': 'Leistungsfaktor L3',
                'value_id': 'PowerFactorL3',
                'start_address': 41,
                'value_type': 'F32BE',
            },
            {
                'name': 'THD-R Spannung L1 [%]',
                'value_id': 'VoltageTHDL1N',
                'start_address': 43,
                'value_type': 'F32BE',
            },
            {
                'name': 'THD-R Spannung L2 [%]',
                'value_id': 'VoltageTHDL2N',
                'start_address': 45,
                'value_type': 'F32BE',
            },
            {
                'name': 'THD-R Spannung L3 [%]',
                'value_id': 'VoltageTHDL3N',
                'start_address': 47,
                'value_type': 'F32BE',
            },
            {
                'name': 'THD-R Strom L1 [%]',
                'value_id': 'CurrentTHDL1',
                'start_address': 49,
                'value_type': 'F32BE',
            },
            {
                'name': 'THD-R Strom L2 [%]',
                'value_id': 'CurrentTHDL2',
                'start_address': 51,
                'value_type': 'F32BE',
            },
            {
                'name': 'THD-R Strom L3 [%]',
                'value_id': 'CurrentTHDL3',
                'start_address': 53,
                'value_type': 'F32BE',
            },
            {
                'name': 'Frequenz [Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 55,
                'value_type': 'F32BE',
            },
            {
                'name': 'Mittelwert Spannung U L-N [V]',
                'value_id': 'VoltageLNAvg',
                'start_address': 57,
                'value_type': 'F32BE',
            },
            {
                'name': 'Mittelwert Spannung U L-L [V]',
                'value_id': 'VoltageLLAvg',
                'start_address': 59,
                'value_type': 'F32BE',
            },
            {
                'name': 'Mittelwert Strom [A]',
                'value_id': 'CurrentLAvgImExDiff',  # FIXME: direction?
                'start_address': 61,
                'value_type': 'F32BE',
            },
            {
                'name': 'Summe der Scheinleistung [VA]',
                'value_id': 'PowerApparentLSumImExDiff',  # FIXME: direction?
                'start_address': 63,
                'value_type': 'F32BE',
            },
            {
                'name': 'Summe der Wirkleistung [W]',
                'value_id': 'PowerActiveLSumImExDiff',  # FIXME: direction?
                'start_address': 65,
                'value_type': 'F32BE',
            },
            {
                'name': 'Summe der Blindleistung [var]',
                'value_id': 'PowerReactiveLSumIndCapDiff',  # FIXME: direction?
                'start_address': 67,
                'value_type': 'F32BE',
            },
            {
                'name': 'Gesamt-Leistungsfaktor',
                'value_id': 'PowerFactorLSum',
                'start_address': 69,
                'value_type': 'F32BE',
            },
            {
                'name': 'Strom N [A]',
                'value_id': 'CurrentNImExDiff',  # FIXME: direction?
                'start_address': 223,
                'value_type': 'F32BE',
            },
            {
                'name': 'Wirkarbeit Bezug Tarif 1 [Wh]',
                'value_id': 'EnergyActiveLSumImportResettable',
                'start_address': 801,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Wirkarbeit Abgabe Tarif 1 [Wh]',
                'value_id': 'EnergyActiveLSumExportResettable',
                'start_address': 809,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Blindarbeit Bezug Tarif 1 [varh]',
                'value_id': 'EnergyReactiveLSumInductiveResettable',  # FIXME: direction?
                'start_address': 817,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Blindarbeit Abgabe Tarif 1 [varh]',
                'value_id': 'EnergyReactiveLSumCapacitiveResettable',  # FIXME: direction?
                'start_address': 825,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Scheinarbeit Tarif 1 [VAh]',
                'value_id': 'EnergyApparentLSumImExSumResettable',  # FIXME: sum?
                'start_address': 833,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L1 Wirkarbeit Bezug Tarif 1 [Wh]',
                'value_id': 'EnergyActiveL1ImportResettable',
                'start_address': 841,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L1 Wirkarbeit Abgabe Tarif 1 [Wh]',
                'value_id': 'EnergyActiveL1ExportResettable',
                'start_address': 849,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L1 Blindarbeit Bezug Tarif 1 [varh]',
                'value_id': 'EnergyReactiveL1InductiveResettable',  # FIXME: direction?
                'start_address': 857,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L1 Blindarbeit Abgabe Tarif 1 [varh]',
                'value_id': 'EnergyReactiveL1CapacitiveResettable',  # FIXME: direction?
                'start_address': 865,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L1 Scheinarbeit Tarif 1 [VAh]',
                'value_id': 'EnergyApparentL1ImExSumResettable',  # FIXME: sum?
                'start_address': 873,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L2 Wirkarbeit Bezug Tarif 1 [Wh]',
                'value_id': 'EnergyActiveL2ImportResettable',
                'start_address': 881,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L2 Wirkarbeit Abgabe Tarif 1 [Wh]',
                'value_id': 'EnergyActiveL2ExportResettable',
                'start_address': 889,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L2 Blindarbeit Bezug Tarif 1 [varh]',
                'value_id': 'EnergyReactiveL2InductiveResettable',  # FIXME: direction?
                'start_address': 897,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L2 Blindarbeit Abgabe Tarif 1 [varh]',
                'value_id': 'EnergyReactiveL2CapacitiveResettable',  # FIXME: direction?
                'start_address': 905,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L2 Scheinarbeit Tarif 1 [VAh]',
                'value_id': 'EnergyApparentL2ImExSumResettable',  # FIXME: sum?
                'start_address': 913,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L3 Wirkarbeit Bezug Tarif 1 [Wh]',
                'value_id': 'EnergyActiveL3ImportResettable',
                'start_address': 921,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L3 Wirkarbeit Abgabe Tarif 1 [Wh]',
                'value_id': 'EnergyActiveL3ExportResettable',
                'start_address': 929,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L3 Blindarbeit Bezug Tarif 1 [varh]',
                'value_id': 'EnergyReactiveL3InductiveResettable',  # FIXME: direction?
                'start_address': 937,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L3 Blindarbeit Abgabe Tarif 1 [varh]',
                'value_id': 'EnergyReactiveL3CapacitiveResettable',  # FIXME: direction?
                'start_address': 945,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L3 Scheinarbeit Tarif 1 [VAh]',
                'value_id': 'EnergyApparentL3ImExSumResettable',  # FIXME: sum?
                'start_address': 953,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
        ],
    },
    {
        'name': 'Siemens PAC3200',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Spannung U L1-N [V]',
                'value_id': 'VoltageL1N',
                'start_address': 1,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung U L2-N [V]',
                'value_id': 'VoltageL2N',
                'start_address': 3,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung U L3-N [V]',
                'value_id': 'VoltageL3N',
                'start_address': 5,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung U L1-L2 [V]',
                'value_id': 'VoltageL1L2',
                'start_address': 7,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung U L2-L3 [V]',
                'value_id': 'VoltageL2L3',
                'start_address': 9,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung U L3-L1 [V]',
                'value_id': 'VoltageL3L1',
                'start_address': 11,
                'value_type': 'F32BE',
            },
            {
                'name': 'Strom L1 [A]',
                'value_id': 'CurrentL1ImExDiff',  # FIXME: direction?
                'start_address': 13,
                'value_type': 'F32BE',
            },
            {
                'name': 'Strom L2 [A]',
                'value_id': 'CurrentL2ImExDiff',  # FIXME: direction?
                'start_address': 15,
                'value_type': 'F32BE',
            },
            {
                'name': 'Strom L3 [A]',
                'value_id': 'CurrentL3ImExDiff',  # FIXME: direction?
                'start_address': 17,
                'value_type': 'F32BE',
            },
            {
                'name': 'Scheinleistung L1 [VA]',
                'value_id': 'PowerApparentL1ImExDiff',  # FIXME: direction?
                'start_address': 19,
                'value_type': 'F32BE',
            },
            {
                'name': 'Scheinleistung L2 [VA]',
                'value_id': 'PowerApparentL2ImExDiff',  # FIXME: direction?
                'start_address': 21,
                'value_type': 'F32BE',
            },
            {
                'name': 'Scheinleistung L3 [VA]',
                'value_id': 'PowerApparentL3ImExDiff',  # FIXME: direction?
                'start_address': 23,
                'value_type': 'F32BE',
            },
            {
                'name': 'Wirkleistung L1 [W]',
                'value_id': 'PowerActiveL1ImExDiff',  # FIXME: direction?
                'start_address': 25,
                'value_type': 'F32BE',
            },
            {
                'name': 'Wirkleistung L2 [W]',
                'value_id': 'PowerActiveL2ImExDiff',  # FIXME: direction?
                'start_address': 27,
                'value_type': 'F32BE',
            },
            {
                'name': 'Wirkleistung L3 [W]',
                'value_id': 'PowerActiveL3ImExDiff',  # FIXME: direction?
                'start_address': 29,
                'value_type': 'F32BE',
            },
            {
                'name': 'Blindleistung L1 [var]',
                'value_id': 'PowerReactiveL1IndCapDiff',  # FIXME: direction?
                'start_address': 31,
                'value_type': 'F32BE',
            },
            {
                'name': 'Blindleistung L2 [var]',
                'value_id': 'PowerReactiveL2IndCapDiff',  # FIXME: direction?
                'start_address': 33,
                'value_type': 'F32BE',
            },
            {
                'name': 'Blindleistung L3 [var]',
                'value_id': 'PowerReactiveL3IndCapDiff',  # FIXME: direction?
                'start_address': 35,
                'value_type': 'F32BE',
            },
            {
                'name': 'Leistungsfaktor L1',
                'value_id': 'PowerFactorL1',
                'start_address': 37,
                'value_type': 'F32BE',
            },
            {
                'name': 'Leistungsfaktor L2',
                'value_id': 'PowerFactorL2',
                'start_address': 39,
                'value_type': 'F32BE',
            },
            {
                'name': 'Leistungsfaktor L3',
                'value_id': 'PowerFactorL3',
                'start_address': 41,
                'value_type': 'F32BE',
            },
            {
                'name': 'THD-R Spannung L1 [%]',
                'value_id': 'VoltageTHDL1N',
                'start_address': 43,
                'value_type': 'F32BE',
            },
            {
                'name': 'THD-R Spannung L2 [%]',
                'value_id': 'VoltageTHDL2N',
                'start_address': 45,
                'value_type': 'F32BE',
            },
            {
                'name': 'THD-R Spannung L3 [%]',
                'value_id': 'VoltageTHDL3N',
                'start_address': 47,
                'value_type': 'F32BE',
            },
            {
                'name': 'THD-R Strom L1 [%]',
                'value_id': 'CurrentTHDL1',
                'start_address': 49,
                'value_type': 'F32BE',
            },
            {
                'name': 'THD-R Strom L2 [%]',
                'value_id': 'CurrentTHDL2',
                'start_address': 51,
                'value_type': 'F32BE',
            },
            {
                'name': 'THD-R Strom L3 [%]',
                'value_id': 'CurrentTHDL3',
                'start_address': 53,
                'value_type': 'F32BE',
            },
            {
                'name': 'Frequenz [Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 55,
                'value_type': 'F32BE',
            },
            {
                'name': 'Mittelwert Spannung U L-N [V]',
                'value_id': 'VoltageLNAvg',
                'start_address': 57,
                'value_type': 'F32BE',
            },
            {
                'name': 'Mittelwert Spannung U L-L [V]',
                'value_id': 'VoltageLLAvg',
                'start_address': 59,
                'value_type': 'F32BE',
            },
            {
                'name': 'Mittelwert Strom [A]',
                'value_id': 'CurrentLAvgImExDiff',  # FIXME: direction?
                'start_address': 61,
                'value_type': 'F32BE',
            },
            {
                'name': 'Summe der Scheinleistung [VA]',
                'value_id': 'PowerApparentLSumImExDiff',  # FIXME: direction?
                'start_address': 63,
                'value_type': 'F32BE',
            },
            {
                'name': 'Summe der Wirkleistung [W]',
                'value_id': 'PowerActiveLSumImExDiff',  # FIXME: direction?
                'start_address': 65,
                'value_type': 'F32BE',
            },
            {
                'name': 'Summe der Blindleistung [var]',
                'value_id': 'PowerReactiveLSumIndCapDiff',  # FIXME: direction?
                'start_address': 67,
                'value_type': 'F32BE',
            },
            {
                'name': 'Gesamt-Leistungsfaktor',
                'value_id': 'PowerFactorLSum',
                'start_address': 69,
                'value_type': 'F32BE',
            },
            {
                'name': 'Wirkarbeit Bezug Tarif 1 [Wh]',
                'value_id': 'EnergyActiveLSumImportResettable',
                'start_address': 801,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Wirkarbeit Abgabe Tarif 1 [Wh]',
                'value_id': 'EnergyActiveLSumExportResettable',
                'start_address': 809,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Blindarbeit Bezug Tarif 1 [varh]',
                'value_id': 'EnergyReactiveLSumInductiveResettable',  # FIXME: direction?
                'start_address': 817,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Blindarbeit Abgabe Tarif 1 [varh]',
                'value_id': 'EnergyReactiveLSumCapacitiveResettable',  # FIXME: direction?
                'start_address': 825,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Scheinarbeit Tarif 1 [VAh]',
                'value_id': 'EnergyApparentLSumImExSumResettable',  # FIXME: sum?
                'start_address': 833,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
        ],
    },
    {
        'name': 'Siemens PAC4200',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Spannung U L1-N [V]',
                'value_id': 'VoltageL1N',
                'start_address': 1,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung U L2-N [V]',
                'value_id': 'VoltageL2N',
                'start_address': 3,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung U L3-N [V]',
                'value_id': 'VoltageL3N',
                'start_address': 5,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung U L1-L2 [V]',
                'value_id': 'VoltageL1L2',
                'start_address': 7,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung U L2-L3 [V]',
                'value_id': 'VoltageL2L3',
                'start_address': 9,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung U L3-L1 [V]',
                'value_id': 'VoltageL3L1',
                'start_address': 11,
                'value_type': 'F32BE',
            },
            {
                'name': 'Strom L1 [A]',
                'value_id': 'CurrentL1ImExDiff',  # FIXME: direction?
                'start_address': 13,
                'value_type': 'F32BE',
            },
            {
                'name': 'Strom L2 [A]',
                'value_id': 'CurrentL2ImExDiff',  # FIXME: direction?
                'start_address': 15,
                'value_type': 'F32BE',
            },
            {
                'name': 'Strom L3 [A]',
                'value_id': 'CurrentL3ImExDiff',  # FIXME: direction?
                'start_address': 17,
                'value_type': 'F32BE',
            },
            {
                'name': 'Scheinleistung L1 [VA]',
                'value_id': 'PowerApparentL1ImExDiff',  # FIXME: direction?
                'start_address': 19,
                'value_type': 'F32BE',
            },
            {
                'name': 'Scheinleistung L2 [VA]',
                'value_id': 'PowerApparentL2ImExDiff',  # FIXME: direction?
                'start_address': 21,
                'value_type': 'F32BE',
            },
            {
                'name': 'Scheinleistung L3 [VA]',
                'value_id': 'PowerApparentL3ImExDiff',  # FIXME: direction?
                'start_address': 23,
                'value_type': 'F32BE',
            },
            {
                'name': 'Wirkleistung L1 [W]',
                'value_id': 'PowerActiveL1ImExDiff',  # FIXME: direction?
                'start_address': 25,
                'value_type': 'F32BE',
            },
            {
                'name': 'Wirkleistung L2 [W]',
                'value_id': 'PowerActiveL2ImExDiff',  # FIXME: direction?
                'start_address': 27,
                'value_type': 'F32BE',
            },
            {
                'name': 'Wirkleistung L3 [W]',
                'value_id': 'PowerActiveL3ImExDiff',  # FIXME: direction?
                'start_address': 29,
                'value_type': 'F32BE',
            },
            {
                'name': 'Blindleistung L1 [var]',
                'value_id': 'PowerReactiveL1IndCapDiff',  # FIXME: direction?
                'start_address': 31,
                'value_type': 'F32BE',
            },
            {
                'name': 'Blindleistung L2 [var]',
                'value_id': 'PowerReactiveL2IndCapDiff',  # FIXME: direction?
                'start_address': 33,
                'value_type': 'F32BE',
            },
            {
                'name': 'Blindleistung L3 [var]',
                'value_id': 'PowerReactiveL3IndCapDiff',  # FIXME: direction?
                'start_address': 35,
                'value_type': 'F32BE',
            },
            {
                'name': 'Leistungsfaktor L1',
                'value_id': 'PowerFactorL1',
                'start_address': 37,
                'value_type': 'F32BE',
            },
            {
                'name': 'Leistungsfaktor L2',
                'value_id': 'PowerFactorL2',
                'start_address': 39,
                'value_type': 'F32BE',
            },
            {
                'name': 'Leistungsfaktor L3',
                'value_id': 'PowerFactorL3',
                'start_address': 41,
                'value_type': 'F32BE',
            },
            {
                'name': 'Frequenz [Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 55,
                'value_type': 'F32BE',
            },
            {
                'name': 'Mittelwert Spannung U L-N [V]',
                'value_id': 'VoltageLNAvg',
                'start_address': 57,
                'value_type': 'F32BE',
            },
            {
                'name': 'Mittelwert Spannung U L-L [V]',
                'value_id': 'VoltageLLAvg',
                'start_address': 59,
                'value_type': 'F32BE',
            },
            {
                'name': 'Mittelwert Strom [A]',
                'value_id': 'CurrentLAvgImExDiff',  # FIXME: direction?
                'start_address': 61,
                'value_type': 'F32BE',
            },
            {
                'name': 'Summe der Scheinleistung [VA]',
                'value_id': 'PowerApparentLSumImExDiff',  # FIXME: direction?
                'start_address': 63,
                'value_type': 'F32BE',
            },
            {
                'name': 'Summe der Wirkleistung [W]',
                'value_id': 'PowerActiveLSumImExDiff',  # FIXME: direction?
                'start_address': 65,
                'value_type': 'F32BE',
            },
            {
                'name': 'Summe der Blindleistung [var]',
                'value_id': 'PowerReactiveLSumIndCapDiff',  # FIXME: direction?
                'start_address': 67,
                'value_type': 'F32BE',
            },
            {
                'name': 'Gesamtleistungsfaktor',
                'value_id': 'PowerFactorLSum',
                'start_address': 69,
                'value_type': 'F32BE',
            },
            {
                'name': 'THD Spannung L1 [%]',
                'value_id': 'VoltageTHDL1N',
                'start_address': 261,
                'value_type': 'F32BE',
            },
            {
                'name': 'THD Spannung L2 [%]',
                'value_id': 'VoltageTHDL2N',
                'start_address': 263,
                'value_type': 'F32BE',
            },
            {
                'name': 'THD Spannung L3 [%]',
                'value_id': 'VoltageTHDL3N',
                'start_address': 265,
                'value_type': 'F32BE',
            },
            {
                'name': 'THD Strom L1 [%]',
                'value_id': 'CurrentTHDL1',
                'start_address': 267,
                'value_type': 'F32BE',
            },
            {
                'name': 'THD Strom L2 [%]',
                'value_id': 'CurrentTHDL2',
                'start_address': 269,
                'value_type': 'F32BE',
            },
            {
                'name': 'THD Strom L3 [%]',
                'value_id': 'CurrentTHDL3',
                'start_address': 271,
                'value_type': 'F32BE',
            },
            {
                'name': 'Neutralleiterstrom [A]',
                'value_id': 'CurrentNImExDiff',  # FIXME: direction?
                'start_address': 295,
                'value_type': 'F32BE',
            },
            {
                'name': 'Bezogene Wirkenergie Tarif 1 [Wh]',
                'value_id': 'EnergyActiveLSumImportResettable',
                'start_address': 801,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Gelieferte Wirkenergie Tarif 1 [Wh]',
                'value_id': 'EnergyActiveLSumExportResettable',
                'start_address': 809,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Bezogene Blindenergie Tarif 1 [varh]',
                'value_id': 'EnergyReactiveLSumInductiveResettable',  # FIXME: direction?
                'start_address': 817,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Gelieferte Blindenergie Tarif 1 [varh]',
                'value_id': 'EnergyReactiveLSumCapacitiveResettable',  # FIXME: direction?
                'start_address': 825,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Scheinenergie Tarif 1 [VAh]',
                'value_id': 'EnergyApparentLSumImExSumResettable',  # FIXME sum?
                'start_address': 833,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
        ],
    },
    {
        'name': 'Siemens PAC4220',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Spannung L1-N [V]',
                'value_id': 'VoltageL1N',
                'start_address': 1,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung L2-N [V]',
                'value_id': 'VoltageL2N',
                'start_address': 3,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung L3-N [V]',
                'value_id': 'VoltageL3N',
                'start_address': 5,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung L1-L2 [V]',
                'value_id': 'VoltageL1L2',
                'start_address': 7,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung L2-L3 [V]',
                'value_id': 'VoltageL2L3',
                'start_address': 9,
                'value_type': 'F32BE',
            },
            {
                'name': 'Spannung L3-L1 [V]',
                'value_id': 'VoltageL3L1',
                'start_address': 11,
                'value_type': 'F32BE',
            },
            {
                'name': 'Strom L1 [A]',
                'value_id': 'CurrentL1ImExDiff',  # FIXME: direction?
                'start_address': 13,
                'value_type': 'F32BE',
            },
            {
                'name': 'Strom L2 [A]',
                'value_id': 'CurrentL2ImExDiff',  # FIXME: direction?
                'start_address': 15,
                'value_type': 'F32BE',
            },
            {
                'name': 'Strom L3 [A]',
                'value_id': 'CurrentL3ImExDiff',  # FIXME: direction?
                'start_address': 17,
                'value_type': 'F32BE',
            },
            {
                'name': 'Scheinleistung L1 [VA]',
                'value_id': 'PowerApparentL1ImExDiff',  # FIXME: direction?
                'start_address': 19,
                'value_type': 'F32BE',
            },
            {
                'name': 'Scheinleistung L2 [VA]',
                'value_id': 'PowerApparentL2ImExDiff',  # FIXME: direction?
                'start_address': 21,
                'value_type': 'F32BE',
            },
            {
                'name': 'Scheinleistung L3 [VA]',
                'value_id': 'PowerApparentL3ImExDiff',  # FIXME: direction?
                'start_address': 23,
                'value_type': 'F32BE',
            },
            {
                'name': 'Wirkleistung L1 [W]',
                'value_id': 'PowerActiveL1ImExDiff',  # FIXME: direction?
                'start_address': 25,
                'value_type': 'F32BE',
            },
            {
                'name': 'Wirkleistung L2 [W]',
                'value_id': 'PowerActiveL2ImExDiff',  # FIXME: direction?
                'start_address': 27,
                'value_type': 'F32BE',
            },
            {
                'name': 'Wirkleistung L3 [W]',
                'value_id': 'PowerActiveL3ImExDiff',  # FIXME: direction?
                'start_address': 29,
                'value_type': 'F32BE',
            },
            {
                'name': 'Leistungsfaktor L1',
                'value_id': 'PowerFactorL1',
                'start_address': 37,
                'value_type': 'F32BE',
            },
            {
                'name': 'Leistungsfaktor L2',
                'value_id': 'PowerFactorL2',
                'start_address': 39,
                'value_type': 'F32BE',
            },
            {
                'name': 'Leistungsfaktor L3',
                'value_id': 'PowerFactorL3',
                'start_address': 41,
                'value_type': 'F32BE',
            },
            {
                'name': 'THD Spannung L1-L2 [%]',
                'value_id': 'VoltageTHDL1L2',
                'start_address': 43,
                'value_type': 'F32BE',
            },
            {
                'name': 'THD Spannung L2-L3 [%]',
                'value_id': 'VoltageTHDL2L3',
                'start_address': 45,
                'value_type': 'F32BE',
            },
            {
                'name': 'THD Spannung L3-L1 [%]',
                'value_id': 'VoltageTHDL3L1',
                'start_address': 47,
                'value_type': 'F32BE',
            },
            {
                'name': 'Frequenz [Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 55,
                'value_type': 'F32BE',
            },
            {
                'name': '3‐Phasen‐Durchschnitt Spannung L-N [V]',
                'value_id': 'VoltageLNAvg',
                'start_address': 57,
                'value_type': 'F32BE',
            },
            {
                'name': '3‐Phasen‐Durchschnitt Spannung L-L [V]',
                'value_id': 'VoltageLLAvg',
                'start_address': 59,
                'value_type': 'F32BE',
            },
            {
                'name': '3‐Phasen‐Durchschnitt Strom [A]',
                'value_id': 'CurrentLAvgImExDiff',  # FIXME: direction?
                'start_address': 61,
                'value_type': 'F32BE',
            },
            {
                'name': 'Gesamtscheinleistung [VA]',
                'value_id': 'PowerApparentLSumImExDiff',  # FIXME: direction?
                'start_address': 63,
                'value_type': 'F32BE',
            },
            {
                'name': 'Gesamtwirkleistung [W]',
                'value_id': 'PowerActiveLSumImExDiff',  # FIXME: direction?
                'start_address': 65,
                'value_type': 'F32BE',
            },
            {
                'name': 'Gesamtwirkleistung [var]',
                'value_id': 'PowerReactiveLSumIndCapDiff',  # FIXME: direction?
                'start_address': 67,
                'value_type': 'F32BE',
            },
            {
                'name': 'Gesamtleistungsfaktor',
                'value_id': 'PowerFactorLSum',
                'start_address': 69,
                'value_type': 'F32BE',
            },
            {
                'name': 'Neutralleiterstrom [A]',
                'value_id': 'CurrentNImExDiff',  # FIXME: direction?
                'start_address': 295,
                'value_type': 'F32BE',
            },
            {
                'name': 'Wirkenergie Bezug Tarif 1 [Wh]',
                'value_id': 'EnergyActiveLSumImportResettable',
                'start_address': 801,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Wirkenergie Abgabe Tarif 1 [Wh]',
                'value_id': 'EnergyActiveLSumExportResettable',
                'start_address': 809,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Blindenergie Bezug Tarif 1 [varh]',
                'value_id': 'EnergyReactiveLSumInductiveResettable',  # FIXME: direction?
                'start_address': 817,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Blindenergie Abgabe Tarif 1 [varh]',
                'value_id': 'EnergyReactiveLSumCapacitiveResettable',  # FIXME: direction?
                'start_address': 825,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Scheinenergie Tarif 1 [VAh]',
                'value_id': 'EnergyApparentLSumImExSumResettable',  # FIXME: sum?
                'start_address': 833,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L1 Wirkenergie Bezug Tarif 1 [Wh]',
                'value_id': 'EnergyActiveL1ImportResettable',
                'start_address': 841,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L1 Wirkenergie Abgabe Tarif 1 [Wh]',
                'value_id': 'EnergyActiveL1ExportResettable',
                'start_address': 849,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L1 Blindenergie Bezug Tarif 1 [varh]',
                'value_id': 'EnergyReactiveL1InductiveResettable',  # FIXME: direction?
                'start_address': 857,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L1 Blindenergie Abgabe Tarif 1 [varh]',
                'value_id': 'EnergyReactiveL1CapacitiveResettable',  # FIXME: direction?
                'start_address': 865,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L1 Scheinenergie Tarif 1 [VAh]',
                'value_id': 'EnergyApparentL1ImExSumResettable',  # FIXME: sum?
                'start_address': 873,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L2 Wirkenergie Bezug Tarif 1 [Wh]',
                'value_id': 'EnergyActiveL2ImportResettable',
                'start_address': 881,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L2 Wirkenergie Abgabe Tarif 1 [Wh]',
                'value_id': 'EnergyActiveL2ExportResettable',
                'start_address': 889,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L2 Blindenergie Bezug Tarif 1 [varh]',
                'value_id': 'EnergyReactiveL2InductiveResettable',  # FIXME: direction?
                'start_address': 897,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L2 Blindenergie Abgabe Tarif 1 [varh]',
                'value_id': 'EnergyReactiveL2CapacitiveResettable',  # FIXME: direction?
                'start_address': 905,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L2 Scheinenergie Tarif 1 [VAh]',
                'value_id': 'EnergyApparentL2ImExSumResettable',  # FIXME: sum?
                'start_address': 913,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L3 Wirkenergie Bezug Tarif 1 [Wh]',
                'value_id': 'EnergyActiveL3ImportResettable',
                'start_address': 921,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L3 Wirkenergie Abgabe Tarif 1 [Wh]',
                'value_id': 'EnergyActiveL3ExportResettable',
                'start_address': 929,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L3 Blindenergie Bezug Tarif 1 [varh]',
                'value_id': 'EnergyReactiveL3InductiveResettable',  # FIXME: direction?
                'start_address': 937,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L3 Blindenergie Abgabe Tarif 1 [varh]',
                'value_id': 'EnergyReactiveL3CapacitiveResettable',  # FIXME: direction?
                'start_address': 945,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
            {
                'name': 'L3 Scheinenergie Tarif 1 [VAh]',
                'value_id': 'EnergyApparentL3ImExSumResettable',  # FIXME: sum?
                'start_address': 953,
                'value_type': 'F64BE',
                'scale_factor': 0.001,
            },
        ],
    },
]
