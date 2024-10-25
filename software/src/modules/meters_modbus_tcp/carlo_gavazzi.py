em100_and_et100_single_phase_values = [
    {
        'name': 'V L-N [0.1 V]',
        'value_id': 'VoltageL1N',
        'start_address': 0x00,
        'value_type': 'S32LE',
        'scale_factor': 0.1,
    },
    {
        'name': 'A [0.001 A]',
        'value_id': 'CurrentL1ImExDiff',  # FIXME: direction?
        'start_address': 0x02,
        'value_type': 'S32LE',
        'scale_factor': 0.001,
    },
    {
        'name': 'W [0.1 W]',
        'value_id': 'PowerActiveL1ImExDiff',  # FIXME: direction?
        'start_address': 0x04,
        'value_type': 'S32LE',
        'scale_factor': 0.1,
    },
    {
        'name': 'W L1+L2+L3 [0.1 W]',
        'value_id': 'PowerActiveLSumImExDiff',
        'start_address': 'START_ADDRESS_VIRTUAL',
    },
    {
        'name': 'VA [0.1 VA]',
        'value_id': 'PowerApparentL1ImExDiff',  # FIXME: direction?
        'start_address': 0x06,
        'value_type': 'S32LE',
        'scale_factor': 0.1,
    },
    {
        'name': 'var [0.1 var]',
        'value_id': 'PowerReactiveL1IndCapDiff',  # FIXME: direction?
        'start_address': 0x08,
        'value_type': 'S32LE',
        'scale_factor': 0.1,
    },
    {
        'name': 'PF [0.001]',
        'value_id': 'PowerFactorL1',
        'start_address': 0x0E,
        'value_type': 'S16',
        'drop_sign': True,  # negative values correspond to capacitive load, positive value correspond to inductive load
        'scale_factor': 0.001,
    },
    {
        'name': 'Hz [0.1 Hz]',
        'value_id': 'FrequencyL1',
        'start_address': 0x0F,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'kWh(+) Total [0.1 kWh]',
        'value_id': 'EnergyActiveL1Import',  # FIXME: direction?
        'start_address': 0x10,
        'value_type': 'S32LE',
        'scale_factor': 0.1,
    },
    {
        'name': 'L1+L2+L3 kWh(+) Total [0.1 kWh]',
        'value_id': 'EnergyActiveLSumImport',
        'start_address': 'START_ADDRESS_VIRTUAL',
    },
    {
        'name': 'kvarh(+) Total [0.1 kvarh]',
        'value_id': 'EnergyReactiveL1Inductive',  # FIXME: direction?
        'start_address': 0x12,
        'value_type': 'S32LE',
        'scale_factor': 0.1,
    },
    {
        'name': 'kWh(-) Total [0.1 kWh]',
        'value_id': 'EnergyActiveL1Export',  # FIXME: direction?
        'start_address': 0x20,
        'value_type': 'S32LE',
        'scale_factor': 0.1,
    },
    {
        'name': 'L1+L2+L3 kWh(-) Total [0.1 kWh]',
        'value_id': 'EnergyActiveLSumExport',
        'start_address': 'START_ADDRESS_VIRTUAL',
    },
    {
        'name': 'kvarh(-) Total [0.1 kvarh]',
        'value_id': 'EnergyReactiveL1Capacitive',  # FIXME: direction?
        'start_address': 0x22,
        'value_type': 'S32LE',
        'scale_factor': 0.1,
    },
]

def make_em100_and_et100_single_phase_values(phase):
    values = []

    for value in em100_and_et100_single_phase_values:
        new_value = dict(value)
        new_value['value_id'] = new_value['value_id'].replace('L1', phase)
        values.append(new_value)

    return values

em510_single_phase_values = [
    {
        'name': 'V L-N [0.1 V]',
        'value_id': 'VoltageL1N',
        'start_address': 0x00,
        'value_type': 'S32LE',
        'scale_factor': 0.1,
    },
    {
        'name': 'A [0.001 A]',
        'value_id': 'CurrentL1ImExDiff',  # FIXME: direction?
        'start_address': 0x02,
        'value_type': 'S32LE',
        'scale_factor': 0.001,
    },
    {
        'name': 'W [0.1 W]',
        'value_id': 'PowerActiveL1ImExDiff',  # FIXME: direction?
        'start_address': 0x04,
        'value_type': 'S32LE',
        'scale_factor': 0.1,
    },
    {
        'name': 'W L1+L2+L3 [0.1 W]',
        'value_id': 'PowerActiveLSumImExDiff',
        'start_address': 'START_ADDRESS_VIRTUAL',
    },
    {
        'name': 'VA [0.1 VA]',
        'value_id': 'PowerApparentL1ImExDiff',  # FIXME: direction?
        'start_address': 0x06,
        'value_type': 'S32LE',
        'scale_factor': 0.1,
    },
    {
        'name': 'var [0.1 var]',
        'value_id': 'PowerReactiveL1IndCapDiff',  # FIXME: direction?
        'start_address': 0x08,
        'value_type': 'S32LE',
        'scale_factor': 0.1,
    },
    {
        'name': 'PF [0.001]',
        'value_id': 'PowerFactorL1Directional',
        'start_address': 0x0E,
        'value_type': 'S16',
        'scale_factor': 0.001,
    },
    {
        'name': 'Hz [0.1 Hz]',
        'value_id': 'FrequencyL1',
        'start_address': 0x0F,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'kWh(+) Total [0.1 kWh]',
        'value_id': 'EnergyActiveL1Import',  # FIXME: direction?
        'start_address': 0x10,
        'value_type': 'S32LE',
        'scale_factor': 0.1,
    },
    {
        'name': 'L1+L2+L3 kWh(+) Total [0.1 kWh]',
        'value_id': 'EnergyActiveLSumImport',
        'start_address': 'START_ADDRESS_VIRTUAL',
    },
    {
        'name': 'kvarh(+) Total [0.1 kvarh]',
        'value_id': 'EnergyReactiveL1Inductive',  # FIXME: direction?
        'start_address': 0x12,
        'value_type': 'S32LE',
        'scale_factor': 0.1,
    },
    {
        'name': 'kWh(-) Total [0.1 kWh]',
        'value_id': 'EnergyActiveL1Export',  # FIXME: direction?
        'start_address': 0x20,
        'value_type': 'S32LE',
        'scale_factor': 0.1,
    },
    {
        'name': 'L1+L2+L3 kWh(-) Total [0.1 kWh]',
        'value_id': 'EnergyActiveLSumExport',
        'start_address': 'START_ADDRESS_VIRTUAL',
    },
    {
        'name': 'kvarh(-) Total [0.1 kvarh]',
        'value_id': 'EnergyReactiveL1Capacitive',  # FIXME: direction?
        'start_address': 0x22,
        'value_type': 'S32LE',
        'scale_factor': 0.1,
    },
    {
        'name': 'kVAh Total [0.1 kVAh]',
        'value_id': 'EnergyApparentL1ImExSum',  # FIXME: sum?
        'start_address': 0x28,
        'value_type': 'S32LE',
        'scale_factor': 0.1,
    },
    {
        'name': 'THD A [0.01 %]',
        'value_id': 'CurrentTHDL1',
        'start_address': 0x32,
        'value_type': 'S32LE',
        'scale_factor': 0.01,
    },
    {
        'name': 'THD V L-N [%]',
        'value_id': 'VoltageTHDL1N',
        'start_address': 0x34,
        'value_type': 'S32LE',
        'scale_factor': 0.01,
    },
]

def make_em510_single_phase_values(phase):
    values = []

    for value in em510_single_phase_values:
        new_value = dict(value)
        new_value['value_id'] = new_value['value_id'].replace('L1', phase)
        values.append(new_value)

    return values

specs = [
    {
        'name': 'Carlo Gavazzi EM24 DIN',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'V L1-N [0.1 V]',
                'value_id': 'VoltageL1N',
                'start_address': 0x00,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L2-N [0.1 V]',
                'value_id': 'VoltageL2N',
                'start_address': 0x02,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L3-N [0.1 V]',
                'value_id': 'VoltageL3N',
                'start_address': 0x04,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L1-L2 [0.1 V]',
                'value_id': 'VoltageL1L2',
                'start_address': 0x06,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L2-L3 [0.1 V]',
                'value_id': 'VoltageL2L3',
                'start_address': 0x08,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L3-L1 [0.1 V]',
                'value_id': 'VoltageL3L1',
                'start_address': 0x0A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'A L1 [0.001 A]',
                'value_id': 'CurrentL1ImExDiff',  # FIXME: direction?
                'start_address': 0x0C,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'A L2 [0.001 A]',
                'value_id': 'CurrentL2ImExDiff',  # FIXME: direction?
                'start_address': 0x0E,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'A L3 [0.001 A]',
                'value_id': 'CurrentL3ImExDiff',  # FIXME: direction?
                'start_address': 0x10,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'W L1 [0.1 W]',
                'value_id': 'PowerActiveL1ImExDiff',  # FIXME: direction?
                'start_address': 0x12,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W L2 [0.1 W]',
                'value_id': 'PowerActiveL2ImExDiff',  # FIXME: direction?
                'start_address': 0x14,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W L3 [0.1 W]',
                'value_id': 'PowerActiveL3ImExDiff',  # FIXME: direction?
                'start_address': 0x16,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA L1 [0.1 VA]',
                'value_id': 'PowerApparentL1ImExDiff',  # FIXME: direction?
                'start_address': 0x18,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA L2 [0.1 VA]',
                'value_id': 'PowerApparentL2ImExDiff',  # FIXME: direction?
                'start_address': 0x1A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA L3 [0.1 VA]',
                'value_id': 'PowerApparentL3ImExDiff',  # FIXME: direction?
                'start_address': 0x1C,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L1 [0.1 var]',
                'value_id': 'PowerReactiveL1IndCapDiff',  # FIXME: direction?
                'start_address': 0x1E,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L2 [0.1 var]',
                'value_id': 'PowerReactiveL2IndCapDiff',  # FIXME: direction?
                'start_address': 0x20,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L3 [0.1 var]',
                'value_id': 'PowerReactiveL3IndCapDiff',  # FIXME: direction?
                'start_address': 0x22,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L-N Sum [0.1 V]',
                'value_id': 'VoltageLNAvg',  # FIXME: datasheet says sum, but average would make more sense
                'start_address': 0x24,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L-L Sum [0.1 V]',
                'value_id': 'VoltageLLAvg',  # FIXME: datasheet says sum, but average would make more sense
                'start_address': 0x26,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W Sum [0.1 W]',
                'value_id': 'PowerActiveLSumImExDiff',  # FIXME: direction?
                'start_address': 0x28,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA Sum [0.1 VA]',
                'value_id': 'PowerApparentLSumImExDiff',  # FIXME: direction?
                'start_address': 0x2A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var Sum [0.1 var]',
                'value_id': 'PowerReactiveLSumIndCapDiff',  # FIXME: direction?
                'start_address': 0x2C,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'PF L1 [0.001]',
                'value_id': 'PowerFactorL1',
                'start_address': 0x32,
                'value_type': 'S16',
                'drop_sign': True,  # negative values correspond to capacitive load, positive value correspond to inductive load
                'scale_factor': 0.001,
            },
            {
                'name': 'PF L2 [0.001]',
                'value_id': 'PowerFactorL2',
                'start_address': 0x33,
                'value_type': 'S16',
                'drop_sign': True,  # negative values correspond to capacitive load, positive value correspond to inductive load
                'scale_factor': 0.001,
            },
            {
                'name': 'PF L3 [0.001]',
                'value_id': 'PowerFactorL3',
                'start_address': 0x34,
                'value_type': 'S16',
                'drop_sign': True,  # negative values correspond to capacitive load, positive value correspond to inductive load
                'scale_factor': 0.001,
            },
            {
                'name': 'PF Sum [0.001]',
                'value_id': 'PowerFactorLSum',  # FIXME: negative values correspond to lead(C), positive value correspond to lag(L)
                'start_address': 0x35,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'Hz [0.1 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 0x37,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) Total [0.1 kWh]',
                'value_id': 'EnergyActiveLSumImport',  # FIXME: direction?
                'start_address': 0x3E,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kvarh(+) Total [0.1 kvarh]',
                'value_id': 'EnergyReactiveLSumInductive',  # FIXME: direction?
                'start_address': 0x40,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) L1 [0.1 kWh]',
                'value_id': 'EnergyActiveL1Import',  # FIXME: direction?
                'start_address': 0x46,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) L2 [0.1 kWh]',
                'value_id': 'EnergyActiveL2Import',  # FIXME: direction?
                'start_address': 0x48,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) L3 [0.1 kWh]',
                'value_id': 'EnergyActiveL3Import',  # FIXME: direction?
                'start_address': 0x4A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(-) Total [0.1 kWh]',
                'value_id': 'EnergyActiveLSumExport',  # FIXME: direction?
                'start_address': 0x5C,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kvarh(-) Total [0.1 kvarh]',
                'value_id': 'EnergyReactiveLSumCapacitive',  # FIXME: direction?
                'start_address': 0x5E,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
        ],
    },
    {
        'name': 'Carlo Gavazzi EM24 E1',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'V L1-N [0.1 V]',
                'value_id': 'VoltageL1N',
                'start_address': 0x00,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L2-N [0.1 V]',
                'value_id': 'VoltageL2N',
                'start_address': 0x02,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L3-N [0.1 V]',
                'value_id': 'VoltageL3N',
                'start_address': 0x04,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L1-L2 [0.1 V]',
                'value_id': 'VoltageL1L2',
                'start_address': 0x06,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L2-L3 [0.1 V]',
                'value_id': 'VoltageL2L3',
                'start_address': 0x08,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L3-L1 [0.1 V]',
                'value_id': 'VoltageL3L1',
                'start_address': 0x0A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'A L1 [0.001 A]',
                'value_id': 'CurrentL1ImExDiff',  # FIXME: direction?
                'start_address': 0x0C,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'A L2 [0.001 A]',
                'value_id': 'CurrentL2ImExDiff',  # FIXME: direction?
                'start_address': 0x0E,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'A L3 [0.001 A]',
                'value_id': 'CurrentL3ImExDiff',  # FIXME: direction?
                'start_address': 0x10,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'W L1 [0.1 W]',
                'value_id': 'PowerActiveL1ImExDiff',  # FIXME: direction?
                'start_address': 0x12,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W L2 [0.1 W]',
                'value_id': 'PowerActiveL2ImExDiff',  # FIXME: direction?
                'start_address': 0x14,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W L3 [0.1 W]',
                'value_id': 'PowerActiveL3ImExDiff',  # FIXME: direction?
                'start_address': 0x16,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA L1 [0.1 VA]',
                'value_id': 'PowerApparentL1ImExDiff',  # FIXME: direction?
                'start_address': 0x18,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA L2 [0.1 VA]',
                'value_id': 'PowerApparentL2ImExDiff',  # FIXME: direction?
                'start_address': 0x1A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA L3 [0.1 VA]',
                'value_id': 'PowerApparentL3ImExDiff',  # FIXME: direction?
                'start_address': 0x1C,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L1 [0.1 var]',
                'value_id': 'PowerReactiveL1IndCapDiff',  # FIXME: direction?
                'start_address': 0x1E,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L2 [0.1 var]',
                'value_id': 'PowerReactiveL2IndCapDiff',  # FIXME: direction?
                'start_address': 0x20,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L3 [0.1 var]',
                'value_id': 'PowerReactiveL3IndCapDiff',  # FIXME: direction?
                'start_address': 0x22,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L-N Sum [0.1 V]',
                'value_id': 'VoltageLNAvg',  # FIXME: datasheet says sum, but average would make more sense
                'start_address': 0x24,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L-L Sum [0.1 V]',
                'value_id': 'VoltageLLAvg',  # FIXME: datasheet says sum, but average would make more sense
                'start_address': 0x26,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W Sum [0.1 W]',
                'value_id': 'PowerActiveLSumImExDiff',  # FIXME: direction?
                'start_address': 0x28,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA Sum [0.1 VA]',
                'value_id': 'PowerApparentLSumImExDiff',  # FIXME: direction?
                'start_address': 0x2A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var Sum [0.1 var]',
                'value_id': 'PowerReactiveLSumIndCapDiff',  # FIXME: direction?
                'start_address': 0x2C,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'PF L1 [0.001]',
                'value_id': 'PowerFactorL1',
                'start_address': 0x2E,
                'value_type': 'S16',
                'drop_sign': True,  # negative values correspond to capacitive load, positive value correspond to inductive load
                'scale_factor': 0.001,
            },
            {
                'name': 'PF L2 [0.001]',
                'value_id': 'PowerFactorL2',
                'start_address': 0x2F,
                'value_type': 'S16',
                'drop_sign': True,  # negative values correspond to capacitive load, positive value correspond to inductive load
                'scale_factor': 0.001,
            },
            {
                'name': 'PF L3 [0.001]',
                'value_id': 'PowerFactorL3',
                'start_address': 0x30,
                'value_type': 'S16',
                'drop_sign': True,  # negative values correspond to capacitive load, positive value correspond to inductive load
                'scale_factor': 0.001,
            },
            {
                'name': 'PF Sum [0.001]',
                'value_id': 'PowerFactorLSum',
                'start_address': 0x31,
                'value_type': 'S16',
                'drop_sign': True,  # negative values correspond to capacitive load, positive value correspond to inductive load
                'scale_factor': 0.001,
            },
            {
                'name': 'Hz [0.1 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 0x33,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) Total [0.1 kWh]',
                'value_id': 'EnergyActiveLSumImport',  # FIXME: direction?
                'start_address': 0x34,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kvarh(+) Total [0.1 kvarh]',
                'value_id': 'EnergyReactiveLSumInductive',  # FIXME: direction?
                'start_address': 0x36,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) L1 [0.1 kWh]',
                'value_id': 'EnergyActiveL1Import',  # FIXME: direction?
                'start_address': 0x40,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) L2 [0.1 kWh]',
                'value_id': 'EnergyActiveL2Import',  # FIXME: direction?
                'start_address': 0x42,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) L3 [0.1 kWh]',
                'value_id': 'EnergyActiveL3Import',  # FIXME: direction?
                'start_address': 0x44,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(-) Total [0.1 kWh]',
                'value_id': 'EnergyActiveLSumExport',  # FIXME: direction?
                'start_address': 0x4E,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kvarh(-) Total [0.1 kvarh]',
                'value_id': 'EnergyReactiveLSumCapacitive',  # FIXME: direction?
                'start_address': 0x50,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
        ],
    },
    {
        'name': 'Carlo Gavazzi EM100 and ET100 At L1',
        'register_type': 'InputRegister',
        'values': make_em100_and_et100_single_phase_values('L1')
    },
    {
        'name': 'Carlo Gavazzi EM100 and ET100 At L2',
        'register_type': 'InputRegister',
        'values': make_em100_and_et100_single_phase_values('L2')
    },
    {
        'name': 'Carlo Gavazzi EM100 and ET100 At L3',
        'register_type': 'InputRegister',
        'values': make_em100_and_et100_single_phase_values('L3')
    },
    {
        'name': 'Carlo Gavazzi EM210',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'V L1-N [0.1 V]',
                'value_id': 'VoltageL1N',
                'start_address': 0x00,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L2-N [0.1 V]',
                'value_id': 'VoltageL2N',
                'start_address': 0x02,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L3-N [0.1 V]',
                'value_id': 'VoltageL3N',
                'start_address': 0x04,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L1-L2 [0.1 V]',
                'value_id': 'VoltageL1L2',
                'start_address': 0x06,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L2-L3 [0.1 V]',
                'value_id': 'VoltageL2L3',
                'start_address': 0x08,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L3-L1 [0.1 V]',
                'value_id': 'VoltageL3L1',
                'start_address': 0x0A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'A L1 [0.001 A]',
                'value_id': 'CurrentL1ImExDiff',  # FIXME: direction?
                'start_address': 0x0C,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'A L2 [0.001 A]',
                'value_id': 'CurrentL2ImExDiff',  # FIXME: direction?
                'start_address': 0x0E,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'A L3 [0.001 A]',
                'value_id': 'CurrentL3ImExDiff',  # FIXME: direction?
                'start_address': 0x10,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'W L1 [0.1 W]',
                'value_id': 'PowerActiveL1ImExDiff',  # FIXME: direction?
                'start_address': 0x12,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W L2 [0.1 W]',
                'value_id': 'PowerActiveL2ImExDiff',  # FIXME: direction?
                'start_address': 0x14,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W L3 [0.1 W]',
                'value_id': 'PowerActiveL3ImExDiff',  # FIXME: direction?
                'start_address': 0x16,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA L1 [0.1 VA]',
                'value_id': 'PowerApparentL1ImExDiff',  # FIXME: direction?
                'start_address': 0x18,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA L2 [0.1 VA]',
                'value_id': 'PowerApparentL2ImExDiff',  # FIXME: direction?
                'start_address': 0x1A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA L3 [0.1 VA]',
                'value_id': 'PowerApparentL3ImExDiff',  # FIXME: direction?
                'start_address': 0x1C,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L1 [0.1 var]',
                'value_id': 'PowerReactiveL1IndCapDiff',  # FIXME: direction?
                'start_address': 0x1E,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L2 [0.1 var]',
                'value_id': 'PowerReactiveL2IndCapDiff',  # FIXME: direction?
                'start_address': 0x20,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L3 [0.1 var]',
                'value_id': 'PowerReactiveL3IndCapDiff',  # FIXME: direction?
                'start_address': 0x22,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L-N Sum [0.1 V]',
                'value_id': 'VoltageLNAvg',  # FIXME: datasheet says sum, but average would make more sense
                'start_address': 0x24,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L-L Sum [0.1 V]',
                'value_id': 'VoltageLLAvg',  # FIXME: datasheet says sum, but average would make more sense
                'start_address': 0x26,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W Sum [0.1 W]',
                'value_id': 'PowerActiveLSumImExDiff',  # FIXME: direction?
                'start_address': 0x28,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA Sum [0.1 VA]',
                'value_id': 'PowerApparentLSumImExDiff',  # FIXME: direction?
                'start_address': 0x2A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var Sum [0.1 var]',
                'value_id': 'PowerReactiveLSumIndCapDiff',  # FIXME: direction?
                'start_address': 0x2C,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'PF L1 [0.001]',
                'value_id': 'PowerFactorL1Directional',
                'start_address': 0x2E,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'PF L2 [0.001]',
                'value_id': 'PowerFactorL2Directional',
                'start_address': 0x2F,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'PF L3 [0.001]',
                'value_id': 'PowerFactorL3Directional',
                'start_address': 0x30,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'PF Sum [0.001]',
                'value_id': 'PowerFactorLSumDirectional',
                'start_address': 0x31,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'Hz [0.1 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 0x33,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) Total [0.1 kWh]',
                'value_id': 'EnergyActiveLSumImport',  # FIXME: direction?
                'start_address': 0x34,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kvarh(+) Total [0.1 kvarh]',
                'value_id': 'EnergyReactiveLSumInductive',  # FIXME: direction?
                'start_address': 0x36,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(-) Total [0.1 kWh]',
                'value_id': 'EnergyActiveLSumExport',  # FIXME: direction?
                'start_address': 0x4E,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
        ],
    },
    {
        'name': 'Carlo Gavazzi EM270 and EM280 Meter',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'V L1-N [0.1 V]',
                'value_id': 'VoltageL1N',
                'start_address': 0x00,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L2-N [0.1 V]',
                'value_id': 'VoltageL2N',
                'start_address': 0x02,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L3-N [0.1 V]',
                'value_id': 'VoltageL3N',
                'start_address': 0x04,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L1-L2 [0.1 V]',
                'value_id': 'VoltageL1L2',
                'start_address': 0x06,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L2-L3 [0.1 V]',
                'value_id': 'VoltageL2L3',
                'start_address': 0x08,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L3-L1 [0.1 V]',
                'value_id': 'VoltageL3L1',
                'start_address': 0x0A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'A L1 [0.001 A]',
                'value_id': 'CurrentL1ImExDiff',  # FIXME: direction?
                'start_address': 0x0C,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'A L2 [0.001 A]',
                'value_id': 'CurrentL2ImExDiff',  # FIXME: direction?
                'start_address': 0x0E,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'A L3 [0.001 A]',
                'value_id': 'CurrentL3ImExDiff',  # FIXME: direction?
                'start_address': 0x10,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'W Sum [0.1 W]',
                'value_id': 'PowerActiveLSumImExDiff',  # FIXME: direction?
                'start_address': 0x22,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA Sum [0.1 VA]',
                'value_id': 'PowerApparentLSumImExDiff',  # FIXME: direction?
                'start_address': 0x14,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var Sum [0.1 var]',
                'value_id': 'PowerReactiveLSumIndCapDiff',  # FIXME: direction?
                'start_address': 0x16,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) Total [0.1 kWh]',
                'value_id': 'EnergyActiveLSumImport',  # FIXME: direction?
                'start_address': 0x18,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kvarh(+) Total [0.1 kvarh]',
                'value_id': 'EnergyReactiveLSumInductive',  # FIXME: direction?
                'start_address': 0x1A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'PF Sum [0.001]',
                'value_id': 'PowerFactorLSumDirectional',
                'start_address': 0x31,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
        ],
    },
    {
        'name': 'Carlo Gavazzi EM270 and EM280 Current Transformer 1',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'A L1 [0.001 A]',
                'value_id': 'CurrentL1ImExDiff',  # FIXME: direction?
                'start_address': 0x010C,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'A L2 [0.001 A]',
                'value_id': 'CurrentL2ImExDiff',  # FIXME: direction?
                'start_address': 0x010E,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'A L3 [0.001 A]',
                'value_id': 'CurrentL3ImExDiff',  # FIXME: direction?
                'start_address': 0x0110,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'W L1 [0.1 W]',
                'value_id': 'PowerActiveL1ImExDiff',  # FIXME: direction?
                'start_address': 0x0112,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W L2 [0.1 W]',
                'value_id': 'PowerActiveL2ImExDiff',  # FIXME: direction?
                'start_address': 0x0114,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W L3 [0.1 W]',
                'value_id': 'PowerActiveL3ImExDiff',  # FIXME: direction?
                'start_address': 0x0116,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W Sum [0.1 W]',
                'value_id': 'PowerActiveLSumImExDiff',  # FIXME: direction?
                'start_address': 0x0118,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA Sum [0.1 VA]',
                'value_id': 'PowerApparentLSumImExDiff',  # FIXME: direction?
                'start_address': 0x011A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var Sum [0.1 var]',
                'value_id': 'PowerReactiveLSumIndCapDiff',  # FIXME: direction?
                'start_address': 0x011C,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) Total [0.1 kWh]',
                'value_id': 'EnergyActiveLSumImport',  # FIXME: direction?
                'start_address': 0x011E,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kvarh(+) Total [0.1 kvarh]',
                'value_id': 'EnergyReactiveLSumInductive',  # FIXME: direction?
                'start_address': 0x0120,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) L1 [0.1 kWh]',
                'value_id': 'EnergyActiveL1Import',  # FIXME: direction?
                'start_address': 0x012A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) L2 [0.1 kWh]',
                'value_id': 'EnergyActiveL2Import',  # FIXME: direction?
                'start_address': 0x012C,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) L3 [0.1 kWh]',
                'value_id': 'EnergyActiveL3Import',  # FIXME: direction?
                'start_address': 0x012E,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L1 [0.1 var]',
                'value_id': 'PowerReactiveL1IndCapDiff',  # FIXME: direction?
                'start_address': 0x013C,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L2 [0.1 var]',
                'value_id': 'PowerReactiveL2IndCapDiff',  # FIXME: direction?
                'start_address': 0x013E,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L3 [0.1 var]',
                'value_id': 'PowerReactiveL3IndCapDiff',  # FIXME: direction?
                'start_address': 0x0140,
                'value_type': 'S32LE',
            },
            {
                'name': 'PF L1 [0.001]',
                'value_id': 'PowerFactorL1Directional',
                'start_address': 0x0142,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'PF L2 [0.001]',
                'value_id': 'PowerFactorL2Directional',
                'start_address': 0x0144,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'PF L3 [0.001]',
                'value_id': 'PowerFactorL3Directional',
                'start_address': 0x0146,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'PF Sum [0.001]',
                'value_id': 'PowerFactorLSumDirectional',
                'start_address': 0x0148,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
        ],
    },
    {
        'name': 'Carlo Gavazzi EM270 and EM280 Current Transformer 2',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'A L1 [0.001 A]',
                'value_id': 'CurrentL1ImExDiff',  # FIXME: direction?
                'start_address': 0x020C,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'A L2 [0.001 A]',
                'value_id': 'CurrentL2ImExDiff',  # FIXME: direction?
                'start_address': 0x020E,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'A L3 [0.001 A]',
                'value_id': 'CurrentL3ImExDiff',  # FIXME: direction?
                'start_address': 0x0210,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'W L1 [0.1 W]',
                'value_id': 'PowerActiveL1ImExDiff',  # FIXME: direction?
                'start_address': 0x0212,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W L2 [0.1 W]',
                'value_id': 'PowerActiveL2ImExDiff',  # FIXME: direction?
                'start_address': 0x0214,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W L3 [0.1 W]',
                'value_id': 'PowerActiveL3ImExDiff',  # FIXME: direction?
                'start_address': 0x0216,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W Sum [0.1 W]',
                'value_id': 'PowerActiveLSumImExDiff',  # FIXME: direction?
                'start_address': 0x0218,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA Sum [0.1 VA]',
                'value_id': 'PowerApparentLSumImExDiff',  # FIXME: direction?
                'start_address': 0x021A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var Sum [0.1 var]',
                'value_id': 'PowerReactiveLSumIndCapDiff',  # FIXME: direction?
                'start_address': 0x021C,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) Total [0.1 kWh]',
                'value_id': 'EnergyActiveLSumImport',  # FIXME: direction?
                'start_address': 0x021E,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kvarh(+) Total [0.1 kvarh]',
                'value_id': 'EnergyReactiveLSumInductive',  # FIXME: direction?
                'start_address': 0x0220,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) L1 [0.1 kWh]',
                'value_id': 'EnergyActiveL1Import',  # FIXME: direction?
                'start_address': 0x022A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) L2 [0.1 kWh]',
                'value_id': 'EnergyActiveL2Import',  # FIXME: direction?
                'start_address': 0x022C,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) L3 [0.1 kWh]',
                'value_id': 'EnergyActiveL3Import',  # FIXME: direction?
                'start_address': 0x022E,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L1 [0.1 var]',
                'value_id': 'PowerReactiveL1IndCapDiff',  # FIXME: direction?
                'start_address': 0x023C,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L2 [0.1 var]',
                'value_id': 'PowerReactiveL2IndCapDiff',  # FIXME: direction?
                'start_address': 0x023E,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L3 [0.1 var]',
                'value_id': 'PowerReactiveL3IndCapDiff',  # FIXME: direction?
                'start_address': 0x0240,
                'value_type': 'S32LE',
            },
            {
                'name': 'PF L1 [0.001]',
                'value_id': 'PowerFactorL1Directional',
                'start_address': 0x0242,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'PF L2 [0.001]',
                'value_id': 'PowerFactorL2Directional',
                'start_address': 0x0244,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'PF L3 [0.001]',
                'value_id': 'PowerFactorL3Directional',
                'start_address': 0x0246,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'PF Sum [0.001]',
                'value_id': 'PowerFactorLSumDirectional',
                'start_address': 0x0248,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
        ],
    },
    {
        'name': 'Carlo Gavazzi EM300',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'V L1-N [0.1 V]',
                'value_id': 'VoltageL1N',
                'start_address': 0x00,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L2-N [0.1 V]',
                'value_id': 'VoltageL2N',
                'start_address': 0x02,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L3-N [0.1 V]',
                'value_id': 'VoltageL3N',
                'start_address': 0x04,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L1-L2 [0.1 V]',
                'value_id': 'VoltageL1L2',
                'start_address': 0x06,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L2-L3 [0.1 V]',
                'value_id': 'VoltageL2L3',
                'start_address': 0x08,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L3-L1 [0.1 V]',
                'value_id': 'VoltageL3L1',
                'start_address': 0x0A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'A L1 [0.001 A]',
                'value_id': 'CurrentL1ImExDiff',  # FIXME: direction?
                'start_address': 0x0C,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'A L2 [0.001 A]',
                'value_id': 'CurrentL2ImExDiff',  # FIXME: direction?
                'start_address': 0x0E,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'A L3 [0.001 A]',
                'value_id': 'CurrentL3ImExDiff',  # FIXME: direction?
                'start_address': 0x10,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'W L1 [0.1 W]',
                'value_id': 'PowerActiveL1ImExDiff',  # FIXME: direction?
                'start_address': 0x12,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W L2 [0.1 W]',
                'value_id': 'PowerActiveL2ImExDiff',  # FIXME: direction?
                'start_address': 0x14,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W L3 [0.1 W]',
                'value_id': 'PowerActiveL3ImExDiff',  # FIXME: direction?
                'start_address': 0x16,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA L1 [0.1 VA]',
                'value_id': 'PowerApparentL1ImExDiff',  # FIXME: direction?
                'start_address': 0x18,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA L2 [0.1 VA]',
                'value_id': 'PowerApparentL2ImExDiff',  # FIXME: direction?
                'start_address': 0x1A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA L3 [0.1 VA]',
                'value_id': 'PowerApparentL3ImExDiff',  # FIXME: direction?
                'start_address': 0x1C,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L1 [0.1 var]',
                'value_id': 'PowerReactiveL1IndCapDiff',  # FIXME: direction?
                'start_address': 0x1E,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L2 [0.1 var]',
                'value_id': 'PowerReactiveL2IndCapDiff',  # FIXME: direction?
                'start_address': 0x20,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L3 [0.1 var]',
                'value_id': 'PowerReactiveL3IndCapDiff',  # FIXME: direction?
                'start_address': 0x22,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L-N Sum [0.1 V]',
                'value_id': 'VoltageLNAvg',  # FIXME: datasheet says sum, but average would make more sense
                'start_address': 0x24,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L-L Sum [0.1 V]',
                'value_id': 'VoltageLLAvg',  # FIXME: datasheet says sum, but average would make more sense
                'start_address': 0x26,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W Sum [0.1 W]',
                'value_id': 'PowerActiveLSumImExDiff',  # FIXME: direction?
                'start_address': 0x28,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA Sum [0.1 VA]',
                'value_id': 'PowerApparentLSumImExDiff',  # FIXME: direction?
                'start_address': 0x2A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var Sum [0.1 var]',
                'value_id': 'PowerReactiveLSumIndCapDiff',  # FIXME: direction?
                'start_address': 0x2C,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'PF L1 [0.001]',
                'value_id': 'PowerFactorL1Directional',
                'start_address': 0x2E,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'PF L2 [0.001]',
                'value_id': 'PowerFactorL2Directional',
                'start_address': 0x2F,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'PF L3 [0.001]',
                'value_id': 'PowerFactorL3Directional',
                'start_address': 0x30,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'PF Sum [0.001]',
                'value_id': 'PowerFactorLSumDirectional',
                'start_address': 0x31,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'Hz [0.1 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 0x33,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) Total [0.1 kWh]',
                'value_id': 'EnergyActiveLSumImport',  # FIXME: direction?
                'start_address': 0x34,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kvarh(+) Total [0.1 kvarh]',
                'value_id': 'EnergyReactiveLSumInductive',  # FIXME: direction?
                'start_address': 0x36,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) L1 [0.1 kWh]',
                'value_id': 'EnergyActiveL1Import',  # FIXME: direction?
                'start_address': 0x40,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) L2 [0.1 kWh]',
                'value_id': 'EnergyActiveL2Import',  # FIXME: direction?
                'start_address': 0x42,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) L3 [0.1 kWh]',
                'value_id': 'EnergyActiveL3Import',  # FIXME: direction?
                'start_address': 0x44,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(-) Total [0.1 kWh]',
                'value_id': 'EnergyActiveLSumExport',  # FIXME: direction?
                'start_address': 0x4E,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kvarh(-) Total [0.1 kvarh]',
                'value_id': 'EnergyReactiveLSumCapacitive',  # FIXME: direction?
                'start_address': 0x50,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
        ],
    },
    {
        'name': 'Carlo Gavazzi ET300',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'V L1-N [0.1 V]',
                'value_id': 'VoltageL1N',
                'start_address': 0x00,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L2-N [0.1 V]',
                'value_id': 'VoltageL2N',
                'start_address': 0x02,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L3-N [0.1 V]',
                'value_id': 'VoltageL3N',
                'start_address': 0x04,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L1-L2 [0.1 V]',
                'value_id': 'VoltageL1L2',
                'start_address': 0x06,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L2-L3 [0.1 V]',
                'value_id': 'VoltageL2L3',
                'start_address': 0x08,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L3-L1 [0.1 V]',
                'value_id': 'VoltageL3L1',
                'start_address': 0x0A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'A L1 [0.001 A]',
                'value_id': 'CurrentL1ImExDiff',  # FIXME: direction?
                'start_address': 0x0C,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'A L2 [0.001 A]',
                'value_id': 'CurrentL2ImExDiff',  # FIXME: direction?
                'start_address': 0x0E,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'A L3 [0.001 A]',
                'value_id': 'CurrentL3ImExDiff',  # FIXME: direction?
                'start_address': 0x10,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'W L1 [0.1 W]',
                'value_id': 'PowerActiveL1ImExDiff',  # FIXME: direction?
                'start_address': 0x12,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W L2 [0.1 W]',
                'value_id': 'PowerActiveL2ImExDiff',  # FIXME: direction?
                'start_address': 0x14,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W L3 [0.1 W]',
                'value_id': 'PowerActiveL3ImExDiff',  # FIXME: direction?
                'start_address': 0x16,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA L1 [0.1 VA]',
                'value_id': 'PowerApparentL1ImExDiff',  # FIXME: direction?
                'start_address': 0x18,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA L2 [0.1 VA]',
                'value_id': 'PowerApparentL2ImExDiff',  # FIXME: direction?
                'start_address': 0x1A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA L3 [0.1 VA]',
                'value_id': 'PowerApparentL3ImExDiff',  # FIXME: direction?
                'start_address': 0x1C,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L1 [0.1 var]',
                'value_id': 'PowerReactiveL1IndCapDiff',  # FIXME: direction?
                'start_address': 0x1E,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L2 [0.1 var]',
                'value_id': 'PowerReactiveL2IndCapDiff',  # FIXME: direction?
                'start_address': 0x20,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L3 [0.1 var]',
                'value_id': 'PowerReactiveL3IndCapDiff',  # FIXME: direction?
                'start_address': 0x22,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L-N Sum [0.1 V]',
                'value_id': 'VoltageLNAvg',  # FIXME: datasheet says sum, but average would make more sense
                'start_address': 0x24,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L-L Sum [0.1 V]',
                'value_id': 'VoltageLLAvg',  # FIXME: datasheet says sum, but average would make more sense
                'start_address': 0x26,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W Sum [0.1 W]',
                'value_id': 'PowerActiveLSumImExDiff',  # FIXME: direction?
                'start_address': 0x28,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA Sum [0.1 VA]',
                'value_id': 'PowerApparentLSumImExDiff',  # FIXME: direction?
                'start_address': 0x2A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var Sum [0.1 var]',
                'value_id': 'PowerReactiveLSumIndCapDiff',  # FIXME: direction?
                'start_address': 0x2C,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'PF L1 [0.001]',
                'value_id': 'PowerFactorL1Directional',
                'start_address': 0x2E,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'PF L2 [0.001]',
                'value_id': 'PowerFactorL2Directional',
                'start_address': 0x2F,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'PF L3 [0.001]',
                'value_id': 'PowerFactorL3Directional',
                'start_address': 0x30,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'PF Sum [0.001]',
                'value_id': 'PowerFactorLSumDirectional',
                'start_address': 0x31,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'Hz [0.1 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 0x33,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) Total [0.1 kWh]',
                'value_id': 'EnergyActiveLSumImport',  # FIXME: direction?
                'start_address': 0x34,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kvarh(+) Total [0.1 kvarh]',
                'value_id': 'EnergyReactiveLSumInductive',  # FIXME: direction?
                'start_address': 0x36,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) L1 [0.1 kWh]',
                'value_id': 'EnergyActiveL1Import',  # FIXME: direction?
                'start_address': 0x40,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) L2 [0.1 kWh]',
                'value_id': 'EnergyActiveL2Import',  # FIXME: direction?
                'start_address': 0x42,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) L3 [0.1 kWh]',
                'value_id': 'EnergyActiveL3Import',  # FIXME: direction?
                'start_address': 0x44,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(-) Total [0.1 kWh]',
                'value_id': 'EnergyActiveLSumExport',  # FIXME: direction?
                'start_address': 0x4E,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kvarh(-) Total [0.1 kvarh]',
                'value_id': 'EnergyReactiveLSumCapacitive',  # FIXME: direction?
                'start_address': 0x50,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(-) L1 [0.1 kWh]',
                'value_id': 'EnergyActiveL1Export',  # FIXME: direction?
                'start_address': 0x60,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(-) L2 [0.1 kWh]',
                'value_id': 'EnergyActiveL2Export',  # FIXME: direction?
                'start_address': 0x62,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(-) L3 [0.1 kWh]',
                'value_id': 'EnergyActiveL3Export',  # FIXME: direction?
                'start_address': 0x64,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
        ],
    },
    {
        'name': 'Carlo Gavazzi EM510 at L1',
        'register_type': 'InputRegister',
        'values': make_em510_single_phase_values('L1')
    },
    {
        'name': 'Carlo Gavazzi EM510 at L2',
        'register_type': 'InputRegister',
        'values': make_em510_single_phase_values('L2')
    },
    {
        'name': 'Carlo Gavazzi EM510 at L3',
        'register_type': 'InputRegister',
        'values': make_em510_single_phase_values('L3')
    },
    {
        'name': 'Carlo Gavazzi EM530 and EM540',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'V L1-N [0.1 V]',
                'value_id': 'VoltageL1N',
                'start_address': 0x00,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L2-N [0.1 V]',
                'value_id': 'VoltageL2N',
                'start_address': 0x02,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L3-N [0.1 V]',
                'value_id': 'VoltageL3N',
                'start_address': 0x04,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L1-L2 [0.1 V]',
                'value_id': 'VoltageL1L2',
                'start_address': 0x06,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L2-L3 [0.1 V]',
                'value_id': 'VoltageL2L3',
                'start_address': 0x08,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L3-L1 [0.1 V]',
                'value_id': 'VoltageL3L1',
                'start_address': 0x0A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'A L1 [0.001 A]',
                'value_id': 'CurrentL1ImExDiff',  # FIXME: direction?
                'start_address': 0x0C,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'A L2 [0.001 A]',
                'value_id': 'CurrentL2ImExDiff',  # FIXME: direction?
                'start_address': 0x0E,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'A L3 [0.001 A]',
                'value_id': 'CurrentL3ImExDiff',  # FIXME: direction?
                'start_address': 0x10,
                'value_type': 'S32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'W L1 [0.1 W]',
                'value_id': 'PowerActiveL1ImExDiff',  # FIXME: direction?
                'start_address': 0x12,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W L2 [0.1 W]',
                'value_id': 'PowerActiveL2ImExDiff',  # FIXME: direction?
                'start_address': 0x14,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W L3 [0.1 W]',
                'value_id': 'PowerActiveL3ImExDiff',  # FIXME: direction?
                'start_address': 0x16,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA L1 [0.1 VA]',
                'value_id': 'PowerApparentL1ImExDiff',  # FIXME: direction?
                'start_address': 0x18,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA L2 [0.1 VA]',
                'value_id': 'PowerApparentL2ImExDiff',  # FIXME: direction?
                'start_address': 0x1A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA L3 [0.1 VA]',
                'value_id': 'PowerApparentL3ImExDiff',  # FIXME: direction?
                'start_address': 0x1C,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L1 [0.1 var]',
                'value_id': 'PowerReactiveL1IndCapDiff',  # FIXME: direction?
                'start_address': 0x1E,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L2 [0.1 var]',
                'value_id': 'PowerReactiveL2IndCapDiff',  # FIXME: direction?
                'start_address': 0x20,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var L3 [0.1 var]',
                'value_id': 'PowerReactiveL3IndCapDiff',  # FIXME: direction?
                'start_address': 0x22,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L-N Sum [0.1 V]',
                'value_id': 'VoltageLNAvg',  # FIXME: datasheet says sum, but average would make more sense
                'start_address': 0x24,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'V L-L Sum [0.1 V]',
                'value_id': 'VoltageLLAvg',  # FIXME: datasheet says sum, but average would make more sense
                'start_address': 0x26,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'W Sum [0.1 W]',
                'value_id': 'PowerActiveLSumImExDiff',  # FIXME: direction?
                'start_address': 0x28,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'VA Sum [0.1 VA]',
                'value_id': 'PowerApparentLSumImExDiff',  # FIXME: direction?
                'start_address': 0x2A,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'var Sum [0.1 var]',
                'value_id': 'PowerReactiveLSumIndCapDiff',  # FIXME: direction?
                'start_address': 0x2C,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'PF L1 [0.001]',
                'value_id': 'PowerFactorL1Directional',
                'start_address': 0x2E,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'PF L2 [0.001]',
                'value_id': 'PowerFactorL2Directional',
                'start_address': 0x2F,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'PF L3 [0.001]',
                'value_id': 'PowerFactorL3Directional',
                'start_address': 0x30,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'PF Sum [0.001]',
                'value_id': 'PowerFactorLSumDirectional',
                'start_address': 0x31,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'Hz [0.1 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 0x33,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) Total [0.1 kWh]',
                'value_id': 'EnergyActiveLSumImport',  # FIXME: direction?
                'start_address': 0x34,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kvarh(+) Total [0.1 kvarh]',
                'value_id': 'EnergyReactiveLSumInductive',  # FIXME: direction?
                'start_address': 0x36,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) L1 [0.1 kWh]',
                'value_id': 'EnergyActiveL1Import',  # FIXME: direction?
                'start_address': 0x40,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) L2 [0.1 kWh]',
                'value_id': 'EnergyActiveL2Import',  # FIXME: direction?
                'start_address': 0x42,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(+) L3 [0.1 kWh]',
                'value_id': 'EnergyActiveL3Import',  # FIXME: direction?
                'start_address': 0x44,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kWh(-) Total [0.1 kWh]',
                'value_id': 'EnergyActiveLSumExport',  # FIXME: direction?
                'start_address': 0x4E,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kvarh(-) Total [0.1 kvarh]',
                'value_id': 'EnergyReactiveLSumCapacitive',  # FIXME: direction?
                'start_address': 0x50,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'kVAh Total [0.1 kVAh]',
                'value_id': 'EnergyApparentLSumImExSum',  # FIXME: sum?
                'start_address': 0x56,
                'value_type': 'S32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'THD A L1 [0.01 %]',
                'value_id': 'CurrentTHDL1',
                'start_address': 0x82,
                'value_type': 'S32LE',
                'scale_factor': 0.01,
            },
            {
                'name': 'THD A L2 [0.01 %]',
                'value_id': 'CurrentTHDL2',
                'start_address': 0x84,
                'value_type': 'S32LE',
                'scale_factor': 0.01,
            },
            {
                'name': 'THD A L3 [0.01 %]',
                'value_id': 'CurrentTHDL3',
                'start_address': 0x86,
                'value_type': 'S32LE',
                'scale_factor': 0.01,
            },
            {
                'name': 'THD V L1-N [%]',
                'value_id': 'VoltageTHDL1N',
                'start_address': 0x8A,
                'value_type': 'S32LE',
                'scale_factor': 0.01,
            },
            {
                'name': 'THD V L2-N [%]',
                'value_id': 'VoltageTHDL2N',
                'start_address': 0x8C,
                'value_type': 'S32LE',
                'scale_factor': 0.01,
            },
            {
                'name': 'THD V L3-N [%]',
                'value_id': 'VoltageTHDL3N',
                'start_address': 0x8E,
                'value_type': 'S32LE',
                'scale_factor': 0.01,
            },
            {
                'name': 'THD V L1-L2 [%]',
                'value_id': 'VoltageTHDL1L2',
                'start_address': 0x92,
                'value_type': 'S32LE',
                'scale_factor': 0.01,
            },
            {
                'name': 'THD V L2-L3 [%]',
                'value_id': 'VoltageTHDL2L3',
                'start_address': 0x94,
                'value_type': 'S32LE',
                'scale_factor': 0.01,
            },
            {
                'name': 'THD V L3-L1 [%]',
                'value_id': 'VoltageTHDL3L1',
                'start_address': 0x96,
                'value_type': 'S32LE',
                'scale_factor': 0.01,
            },
        ],
    },
]
