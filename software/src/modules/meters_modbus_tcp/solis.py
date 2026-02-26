table_prototypes = [
    ('Solis Hybrid Inverter', ['device_address', 'virtual_meter']),
]

default_device_addresses = [
    ('Solis Hybrid Inverter', 1),
]

specs = [
    {
        'name': 'Solis Hybrid Inverter',
        'virtual_meter': ('Solis Hybrid Inverter', 'Inverter'),
        'default_location': 'Inverter',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Phase A voltage [0.1 V]',
                'value_id': 'VoltageL1N',
                'start_address': 33073,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Phase B voltage [0.1 V]',
                'value_id': 'VoltageL2N',
                'start_address': 33074,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Phase C voltage [0.1 V]',
                'value_id': 'VoltageL3N',
                'start_address': 33075,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Phase A current [0.1 A]',
                'value_id': 'CurrentL1ImExSum',
                'start_address': 33076,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Phase B current [0.1 A]',
                'value_id': 'CurrentL2ImExSum',
                'start_address': 33077,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Phase C current [0.1 A]',
                'value_id': 'CurrentL3ImExSum',
                'start_address': 33078,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Active power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 33079,
                'value_type': 'S32BE',
                'scale_factor': -1.0,
            },
            {
                'name': 'Reactive power [var]',
                'value_id': 'PowerReactiveLSumIndCapSum',  # FIXME: sign?
                'start_address': 33081,
                'value_type': 'S32BE',
            },
            {
                'name': 'Apparent power [VA]',
                'value_id': 'PowerApparentLSumImExSum',  # FIXME: sign?
                'start_address': 33083,
                'value_type': 'S32BE',
            },
            {
                'name': 'Inverter temperature [0.1 °C]',
                'value_id': 'Temperature',
                'start_address': 33093,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Grid frequency [0.01 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 33094,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
        ],
    },
    {
        'name': 'Solis Hybrid Inverter Grid',
        'virtual_meter': ('Solis Hybrid Inverter', 'Grid'),
        'default_location': 'Grid',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Meter AC voltage A [0.1 V]',
                'value_id': 'VoltageL1N',
                'start_address': 33251,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Meter AC current A [0.01 A]',
                'value_id': 'CurrentL1ImExSum',
                'start_address': 33252,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
            {
                'name': 'Meter AC voltage B [0.1 V]',
                'value_id': 'VoltageL2N',
                'start_address': 33253,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Meter AC current B [0.01 A]',
                'value_id': 'CurrentL2ImExSum',
                'start_address': 33254,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
            {
                'name': 'Meter AC voltage C [0.1 V]',
                'value_id': 'VoltageL3N',
                'start_address': 33255,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Meter AC current C [0.01 A]',
                'value_id': 'CurrentL3ImExSum',
                'start_address': 33256,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
            {
                'name': 'Meter active power A [W]',
                'value_id': 'PowerActiveL1ImExDiff',
                'start_address': 33257,
                'value_type': 'S32BE',
                'scale_factor': -1.0,
            },
            {
                'name': 'Meter active power B [W]',
                'value_id': 'PowerActiveL2ImExDiff',
                'start_address': 33259,
                'value_type': 'S32BE',
                'scale_factor': -1.0,
            },
            {
                'name': 'Meter active power C [W]',
                'value_id': 'PowerActiveL3ImExDiff',
                'start_address': 33261,
                'value_type': 'S32BE',
                'scale_factor': -1.0,
            },
            {
                'name': 'Meter total active power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 33263,
                'value_type': 'S32BE',
                'scale_factor': -1.0,
            },
            {
                'name': 'Meter reactive power A [var]',
                'value_id': 'PowerReactiveL1IndCapSum',  # FIXME: sign?
                'start_address': 33265,
                'value_type': 'S32BE',
            },
            {
                'name': 'Meter reactive power B [var]',
                'value_id': 'PowerReactiveL2IndCapSum',  # FIXME: sign?
                'start_address': 33267,
                'value_type': 'S32BE',
            },
            {
                'name': 'Meter reactive power C [var]',
                'value_id': 'PowerReactiveL3IndCapSum',  # FIXME: sign?
                'start_address': 33269,
                'value_type': 'S32BE',
            },
            {
                'name': 'Meter total reactive power [var]',
                'value_id': 'PowerReactiveLSumIndCapSum',  # FIXME: sign?
                'start_address': 33271,
                'value_type': 'S32BE',
            },
            {
                'name': 'Meter apparent power A [VA]',
                'value_id': 'PowerApparentL1ImExSum',  # FIXME: sign?
                'start_address': 33273,
                'value_type': 'S32BE',
            },
            {
                'name': 'Meter apparent power B [VA]',
                'value_id': 'PowerApparentL2ImExSum',  # FIXME: sign?
                'start_address': 33275,
                'value_type': 'S32BE',
            },
            {
                'name': 'Meter apparent power C [VA]',
                'value_id': 'PowerApparentL3ImExSum',  # FIXME: sign?
                'start_address': 33277,
                'value_type': 'S32BE',
            },
            {
                'name': 'Meter total apparent power [VA]',
                'value_id': 'PowerApparentLSumImExSum',  # FIXME: sign?
                'start_address': 33279,
                'value_type': 'S32BE',
            },
            {
                'name': 'Meter power factor [0.01]',
                'value_id': 'PowerFactorLSum',
                'start_address': 33281,
                'value_type': 'S16',
                'scale_factor': -0.01,
            },
            {
                'name': 'Meter grid frequency [0.01 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 33282,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
            {
                'name': 'Meter total active energy from grid [0.01 kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 33283,
                'value_type': 'U32BE',
                'scale_factor': 0.01,
            },
            {
                'name': 'Meter total active energy to grid [0.01 kWh]',
                'value_id': 'EnergyActiveLSumExport',
                'start_address': 33285,
                'value_type': 'U32BE',
                'scale_factor': 0.01,
            },
        ],
    },
    {
        'name': 'Solis Hybrid Inverter Battery',
        'virtual_meter': ('Solis Hybrid Inverter', 'Battery'),
        'default_location': 'Battery',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Battery voltage [0.1 V]',
                'value_id': 'VoltageDC',
                'start_address': 33133,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery current [0.1 A]',
                'value_id': 'VALUE_ID_META',
                'start_address': 33134,
                'value_type': 'S16',  # value is acutually unsigned
            },
            {
                'name': 'Battery current direction',
                'value_id': 'VALUE_ID_META',
                'start_address': 33135,
                'value_type': 'U16',
            },
            {
                'name': 'Battery current signed [0.1 A]',
                'value_id': 'CurrentDCChaDisDiff',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
            {
                'name': 'Battery capacity SOC [%]',
                'value_id': 'StateOfCharge',
                'start_address': 33139,
                'value_type': 'U16',
            },
            {
                'name': 'Battery power [W]',
                'value_id': 'VALUE_ID_META',
                'start_address': 33149,
                'value_type': 'S32BE',  # value is acutually unsigned
            },
            {
                'name': 'Battery power signed [W]',
                'value_id': 'PowerDCChaDisDiff',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
            {
                'name': 'Total battery charge [kWh]',
                'value_id': 'EnergyDCCharge',
                'start_address': 33161,
                'value_type': 'U32BE',
            },
            {
                'name': 'Total battery discharge [kWh]',
                'value_id': 'EnergyDCDischarge',
                'start_address': 33165,
                'value_type': 'U32BE',
            },
        ],
    },
    {
        'name': 'Solis Hybrid Inverter Load',
        'virtual_meter': ('Solis Hybrid Inverter', 'Load'),
        'default_location': 'Load',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'House load power [W]',
                'value_id': 'VALUE_ID_META',
                'start_address': 33147,
                'value_type': 'U16',
            },
            {
                'name': 'Backup load power [W]',
                'value_id': 'VALUE_ID_META',
                'start_address': 33148,
                'value_type': 'U16',
            },
            {
                'name': 'Total load power [W]',
                'value_id': 'PowerActiveLSumImport',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
            {
                'name': 'Total load power signed [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
            {
                'name': 'Total load energy consumption [kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 33177,
                'value_type': 'U32BE',
            },
        ],
    },
    {
        'name': 'Solis Hybrid Inverter PV',
        'virtual_meter': ('Solis Hybrid Inverter', 'PV'),
        'default_location': 'PV',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Inverter total power generation [kWh]',
                'value_id': 'EnergyPVSumExport',
                'start_address': 33029,
                'value_type': 'U32BE',
            },
            {
                'name': 'DC voltage 1 [0.1 V]',
                'value_id': 'VoltagePV1',
                'start_address': 33049,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'DC current 1 [0.1 A]',
                'value_id': 'CurrentPV1Export',
                'start_address': 33050,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'DC voltage 2 [0.1 V]',
                'value_id': 'VoltagePV2',
                'start_address': 33051,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'DC current 2 [0.1 A]',
                'value_id': 'CurrentPV2Export',
                'start_address': 33052,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'DC voltage 3 [0.1 V]',
                'value_id': 'VoltagePV3',
                'start_address': 33053,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'DC current 3 [0.1 A]',
                'value_id': 'CurrentPV3Export',
                'start_address': 33054,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'DC voltage 4 [0.1 V]',
                'value_id': 'VoltagePV4',
                'start_address': 33055,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'DC current 4 [0.1 A]',
                'value_id': 'CurrentPV4Export',
                'start_address': 33056,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Average DC voltage [0.1 V]',
                'value_id': 'VoltagePVAvg',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
            {
                'name': 'Total DC current [0.1 A]',
                'value_id': 'CurrentPVSumExport',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
            {
                'name': 'Total DC output power [W]',
                'value_id': 'PowerPVSumExport',
                'start_address': 33057,
                'value_type': 'U32BE',
            },
        ],
    },
]
