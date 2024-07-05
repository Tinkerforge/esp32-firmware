monophase_values = [
    {
        'name': 'Channel 1 voltage [V]',
        'value_id': 'VoltageL1N',
        'start_address': 2003,
        'value_type': 'F32LE',
    },
    {
        'name': 'Channel 1 current [A]',
        'value_id': 'CurrentL1ImExSum',
        'start_address': 2005,
        'value_type': 'F32LE',
    },
    {
        'name': 'Channel 1 active power [W]',
        'value_id': 'PowerActiveL1ImExDiff',
        'start_address': 2007,
        'value_type': 'F32LE',
    },
    {
        'name': 'Channel 1 apparent power [VA]',
        'value_id': 'PowerApparentL1ImExSum',
        'start_address': 2009,
        'value_type': 'F32LE',
    },
    {
        'name': 'Channel 1 power factor',
        'value_id': 'PowerFactorL1',
        'start_address': 2011,
        'value_type': 'F32LE',
    },
    {
        'name': 'Channel 1 total active energy - perpetual count [Wh]',
        'value_id': 'EnergyActiveL1ImportResettable',
        'start_address': 2310,
        'value_type': 'F32LE',
        'scale_factor': 0.001
    },
    {
        'name': 'Channel 1 total active returned energy - perpetual count [Wh]',
        'value_id': 'EnergyActiveL1ExportResettable',
        'start_address': 2312,
        'value_type': 'F32LE',
        'scale_factor': 0.001
    },
]


def make_monophase_values(channel, phase, start_address_offset):
    values = []

    for value in monophase_values:
        new_value = dict(value)

        new_value['name'] = new_value['name'].replace('Channel 1', channel)
        new_value['value_id'] = new_value['value_id'].replace('L1', phase)
        new_value['start_address'] += start_address_offset

        values.append(new_value)

    return values


specs = [
    {
        'name': 'Shelly EM Triphase',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Neutral current [A]',
                'value_id': 'CurrentNImExSum',
                'start_address': 1007,
                'value_type': 'F32LE',
            },
            {
                'name': 'Total current [A]',
                'value_id': 'CurrentLSumImExSum',
                'start_address': 1011,
                'value_type': 'F32LE',
            },
            {
                'name': 'Total active power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 1013,
                'value_type': 'F32LE',
            },
            {
                'name': 'Total apparent power [VA]',
                'value_id': 'PowerApparentLSumImExSum',
                'start_address': 1015,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase A voltage [V]',
                'value_id': 'VoltageL1N',
                'start_address': 1020,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase A current [A]',
                'value_id': 'CurrentL1ImExSum',
                'start_address': 1022,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase A active power [W]',
                'value_id': 'PowerActiveL1ImExDiff',
                'start_address': 1024,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase A apparent power [VA]',
                'value_id': 'PowerApparentL1ImExSum',
                'start_address': 1026,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase A power factor',
                'value_id': 'PowerFactorL1',
                'start_address': 1028,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase B voltage [V]',
                'value_id': 'VoltageL2N',
                'start_address': 1040,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase B current [A]',
                'value_id': 'CurrentL2ImExSum',
                'start_address': 1042,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase B active power [W]',
                'value_id': 'PowerActiveL2ImExDiff',
                'start_address': 1044,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase B apparent power [VA]',
                'value_id': 'PowerApparentL2ImExSum',
                'start_address': 1046,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase B power factor',
                'value_id': 'PowerFactorL2',
                'start_address': 1048,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase C voltage [V]',
                'value_id': 'VoltageL3N',
                'start_address': 1060,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase C current [A]',
                'value_id': 'CurrentL3ImExSum',
                'start_address': 1062,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase C active power [W]',
                'value_id': 'PowerActiveL3ImExDiff',
                'start_address': 1064,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase C apparent power [VA]',
                'value_id': 'PowerApparentL3ImExSum',
                'start_address': 1066,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase C power factor',
                'value_id': 'PowerFactorL3',
                'start_address': 1068,
                'value_type': 'F32LE',
            },
            {
                'name': 'Total active energy accumulated for all phases - perpetual count [Wh]',
                'value_id': 'EnergyActiveLSumImportResettable',
                'start_address': 1162,
                'value_type': 'F32LE',
                'scale_factor': 0.001
            },
            {
                'name': 'Total active returned energy accumulated for all phases - perpetual count [Wh]',
                'value_id': 'EnergyActiveLSumExportResettable',
                'start_address': 1164,
                'value_type': 'F32LE',
                'scale_factor': 0.001
            },
            {
                'name': 'Phase A total active energy - perpetual count [Wh]',
                'value_id': 'EnergyActiveL1ImportResettable',
                'start_address': 1182,
                'value_type': 'F32LE',
                'scale_factor': 0.001
            },
            {
                'name': 'Phase A total active returned energy - perpetual count [Wh]',
                'value_id': 'EnergyActiveL1ExportResettable',
                'start_address': 1184,
                'value_type': 'F32LE',
                'scale_factor': 0.001
            },
            {
                'name': 'Phase B total active energy - perpetual count [Wh]',
                'value_id': 'EnergyActiveL2ImportResettable',
                'start_address': 1202,
                'value_type': 'F32LE',
                'scale_factor': 0.001
            },
            {
                'name': 'Phase B total active returned energy - perpetual count [Wh]',
                'value_id': 'EnergyActiveL2ExportResettable',
                'start_address': 1204,
                'value_type': 'F32LE',
                'scale_factor': 0.001
            },
            {
                'name': 'Phase C total active energy - perpetual count [Wh]',
                'value_id': 'EnergyActiveL3ImportResettable',
                'start_address': 1222,
                'value_type': 'F32LE',
                'scale_factor': 0.001
            },
            {
                'name': 'Phase C total active returned energy - perpetual count [Wh]',
                'value_id': 'EnergyActiveL3ExportResettable',
                'start_address': 1224,
                'value_type': 'F32LE',
                'scale_factor': 0.001
            },
        ],
    },
    {
        'name': 'Shelly EM Monophase Channel 1 As L1',
        'register_type': 'InputRegister',
        'values': make_monophase_values('Channel 1', 'L1', 0)
    },
    {
        'name': 'Shelly EM Monophase Channel 1 As L2',
        'register_type': 'InputRegister',
        'values': make_monophase_values('Channel 1', 'L2', 0)
    },
    {
        'name': 'Shelly EM Monophase Channel 1 As L3',
        'register_type': 'InputRegister',
        'values': make_monophase_values('Channel 1', 'L3', 0)
    },
    {
        'name': 'Shelly EM Monophase Channel 2 As L1',
        'register_type': 'InputRegister',
        'values': make_monophase_values('Channel 2', 'L1', 20)
    },
    {
        'name': 'Shelly EM Monophase Channel 2 As L2',
        'register_type': 'InputRegister',
        'values': make_monophase_values('Channel 2', 'L2', 20)
    },
    {
        'name': 'Shelly EM Monophase Channel 2 As L3',
        'register_type': 'InputRegister',
        'values': make_monophase_values('Channel 2', 'L3', 20)
    },
    {
        'name': 'Shelly EM Monophase Channel 3 As L1',
        'register_type': 'InputRegister',
        'values': make_monophase_values('Channel 3', 'L1', 40)
    },
    {
        'name': 'Shelly EM Monophase Channel 3 As L2',
        'register_type': 'InputRegister',
        'values': make_monophase_values('Channel 3', 'L2', 40)
    },
    {
        'name': 'Shelly EM Monophase Channel 3 As L3',
        'register_type': 'InputRegister',
        'values': make_monophase_values('Channel 3', 'L3', 40)
    },
]
