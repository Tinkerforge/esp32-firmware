specs = [
    {
        'name': 'Huawei SUN2000 Inverter',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Power grid voltage or line voltage between phases A and B [0.1 V]',
                'value_id': 'VoltageL1L2',
                'start_address': 32066,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Line voltage between phases B and C [0.1 V]',
                'value_id': 'VoltageL2L3',
                'start_address': 32067,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Line voltage between phases C and A [0.1 V]',
                'value_id': 'VoltageL3L1',
                'start_address': 32068,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Phase A voltage [0.1 V]',
                'value_id': 'VoltageL1N',
                'start_address': 32069,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Phase B voltage [0.1 V]',
                'value_id': 'VoltageL2N',
                'start_address': 32070,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Phase C voltage [0.1 V]',
                'value_id': 'VoltageL3N',
                'start_address': 32071,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Power grid current or phase A current [0.001 A]',
                'value_id': 'CurrentL1ImExDiff',  # FIXME: direction?
                'start_address': 32072,
                'value_type': 'S32BE',
                'scale_factor': -0.001,
            },
            {
                'name': 'Phase B current [0.001 A]',
                'value_id': 'CurrentL2ImExDiff',  # FIXME: direction?
                'start_address': 32074,
                'value_type': 'S32BE',
                'scale_factor': -0.001,
            },
            {
                'name': 'Phase C current [0.001 A]',
                'value_id': 'CurrentL3ImExDiff',  # FIXME: direction?
                'start_address': 32076,
                'value_type': 'S32BE',
                'scale_factor': -0.001,
            },
            {
                'name': 'Active power [W]',
                'value_id': 'PowerActiveLSumImExDiff',  # FIXME: direction?
                'start_address': 32080,
                'value_type': 'S32BE',
                'scale_factor': -1.0,
            },
            {
                'name': 'Reactive power [var]',
                'value_id': 'PowerReactiveLSumIndCapDiff',  # FIXME: direction?
                'start_address': 32082,
                'value_type': 'S32BE',
            },
            {
                'name': 'Power factor [0.001]',
                'value_id': 'PowerFactorLSumDirectional',  # FIXME: direction?
                'start_address': 32084,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'Grid frequency [0.01 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 32085,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
            {
                'name': 'Internal temperature [0.1 °C]',
                'value_id': 'Temperature',
                'start_address': 32087,
                'value_type': 'S16',
                'scale_factor': 0.1,
            },
        ],
    },
    {
        'name': 'Huawei SUN2000 Grid',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Grid voltage A phase [0.1 V]',
                'value_id': 'VoltageL1N',
                'start_address': 37101,
                'value_type': 'S32BE',
                'scale_factor': 0.1,
            },
            {
                'name': 'B phase voltage [0.1 V]',
                'value_id': 'VoltageL2N',
                'start_address': 37103,
                'value_type': 'S32BE',
                'scale_factor': 0.1,
            },
            {
                'name': 'C phase voltage [0.1 V]',
                'value_id': 'VoltageL3N',
                'start_address': 37105,
                'value_type': 'S32BE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Grid current A phase [0.01 A]',
                'value_id': 'CurrentL1ImExDiff',
                'start_address': 37107,
                'value_type': 'S32BE',
                'scale_factor': -0.01,
            },
            {
                'name': 'B phase current [0.01 A]',
                'value_id': 'CurrentL2ImExDiff',
                'start_address': 37109,
                'value_type': 'S32BE',
                'scale_factor': -0.01,
            },
            {
                'name': 'C phase current [0.01 A]',
                'value_id': 'CurrentL3ImExDiff',
                'start_address': 37111,
                'value_type': 'S32BE',
                'scale_factor': -0.01,
            },
            {
                'name': 'Active power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 37113,
                'value_type': 'S32BE',
                'scale_factor': -1.0,
            },
            {
                'name': 'Reactive power [var]',
                'value_id': 'PowerReactiveLSumIndCapDiff',  # FIXME: direction?
                'start_address': 37115,
                'value_type': 'S32BE',
            },
            {
                'name': 'Power factor [0.001]',
                'value_id': 'PowerFactorLSumDirectional',  # FIXME: direction?
                'start_address': 37117,
                'value_type': 'S16',
                'scale_factor': 0.001,
            },
            {
                'name': 'Grid frequency [0.01 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 37118,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
            {
                'name': 'Positive active electricity [0.01 kWh]',
                'value_id': 'EnergyActiveLSumExport',
                'start_address': 37119,
                'value_type': 'S32BE',
                'scale_factor': 0.01,
            },
            {
                'name': 'Reverse active power [0.01 kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 37121,
                'value_type': 'S32BE',
                'scale_factor': 0.01,
            },
            {
                'name': 'Accumulated reactive power [0.01 kvarh]',
                'value_id': 'EnergyReactiveLSumIndCapDiff',  # FIXME: direction?
                'start_address': 37123,
                'value_type': 'S32BE',
                'scale_factor': 0.01,
            },
            {
                'name': 'A-B line voltage [0.1 V]',
                'value_id': 'VoltageL1L2',
                'start_address': 37126,
                'value_type': 'S32BE',
                'scale_factor': 0.1,
            },
            {
                'name': 'B-C line voltage [0.1 V]',
                'value_id': 'VoltageL2L3',
                'start_address': 37126,
                'value_type': 'S32BE',
                'scale_factor': 0.1,
            },
            {
                'name': 'C-A line voltage [0.1 V]',
                'value_id': 'VoltageL3L1',
                'start_address': 37126,
                'value_type': 'S32BE',
                'scale_factor': 0.1,
            },
            {
                'name': 'A phase active power [W]',
                'value_id': 'PowerActiveL1ImExDiff',
                'start_address': 37132,
                'value_type': 'S32BE',
                'scale_factor': -1.0,
            },
            {
                'name': 'B phase active power [W]',
                'value_id': 'PowerActiveL2ImExDiff',
                'start_address': 37134,
                'value_type': 'S32BE',
                'scale_factor': -1.0,
            },
            {
                'name': 'C phase active power [W]',
                'value_id': 'PowerActiveL3ImExDiff',
                'start_address': 37136,
                'value_type': 'S32BE',
                'scale_factor': -1.0,
            },
        ],
    },
    {
        'name': 'Huawei SUN2000 Battery Product Model',
        'register_type': 'HoldingRegister',
        'values': [
            {
                # FIXME: which are valid combinations for unit 1/2?
                #
                #        LUNA2000
                #        LUNA2000/LUNA2000
                #        LG-RESU
                #        LG-RESU/LG-RESU (invalid, documentation says only one LG-RESU can be connected)
                #        LUNA2000/LG-RESU (assumed to be invalid)
                #        LG-RESU/LUNA2000 (assumed to be invalid)
                #
                #        with this assumptions it should be okay to just read the combined product
                #        model 47106, instead of reading the unit 1/2 product model 47000/47089
                'name': 'Energy storage - product model',
                'value_id': 'VALUE_ID_META',
                'start_address': 47106,
                'value_type': 'U16',
            },
        ],
    },
    {
        'name': 'Huawei SUN2000 Battery Huawei LUNA2000',  # two can be connected, use combined registers
        'register_type': 'HoldingRegister',
        'values': [
            {
                # FIXME: there is no combined temperature register. just use unit 1 temperature for
                #        now. could read unit 1/2 running status 37000/37741 and or unit 1/2 product
                #        model 47000/47089/47106 and manually combine unit 1/2 temperature registers
                #        37022/37752
                'name': 'Energy storage unit 1 - battery temperature [0.1 °C]',
                'value_id': 'Temperature',
                'start_address': 37022,
                'value_type': 'S16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Energy storage - SOC [0.1 %]',
                'value_id': 'StateOfCharge',
                'start_address': 37760,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Energy storage - bus voltage [0.1 V]',
                'value_id': 'VoltageDC',
                'start_address': 37763,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Energy storage - bus current [0.1 A]',
                'value_id': 'CurrentDCChaDisDiff',
                'start_address': 37764,
                'value_type': 'S16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Energy storage - charge discharge power [W]',
                'value_id': 'PowerDCChaDisDiff',
                'start_address': 37765,
                'value_type': 'S32BE',
            },
            {
                'name': 'Energy storage - total charge [0.01 kWh]',
                'value_id': 'EnergyDCCharge',
                'start_address': 37780,
                'value_type': 'U32BE',
                'scale_factor': 0.01,
            },
            {
                'name': 'Energy storage - total discharge [0.01 kWh]',
                'value_id': 'EnergyDCDischarge',
                'start_address': 37782,
                'value_type': 'U32BE',
                'scale_factor': 0.01,
            },
        ],
    },
    {
        'name': 'Huawei SUN2000 Battery LG RESU',  # only one can be connected, use unit 1 registers
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Energy storage unit 1 - charge discharge power [W]',
                'value_id': 'PowerDCChaDisDiff',
                'start_address': 37001,
                'value_type': 'S32BE',
            },
            {
                'name': 'Energy storage unit 1 - bus voltage [0.1 V]',
                'value_id': 'VoltageDC',
                'start_address': 37003,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Energy storage unit 1 - SOC [0.1 %]',
                'value_id': 'StateOfCharge',
                'start_address': 37004,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Energy storage unit 1 - bus current [0.1 A]',
                'value_id': 'CurrentDCChaDisDiff',
                'start_address': 37021,
                'value_type': 'S16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Energy storage unit 1 - battery temperature [0.1 °C]',
                'value_id': 'Temperature',
                'start_address': 37022,
                'value_type': 'S16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Energy storage unit 1 - total charge [0.01 kWh]',
                'value_id': 'EnergyDCCharge',
                'start_address': 37066,
                'value_type': 'U32BE',
                'scale_factor': 0.01,
            },
            {
                'name': 'Energy storage unit 1 - total discharge [0.01 kWh]',
                'value_id': 'EnergyDCDischarge',
                'start_address': 37068,
                'value_type': 'U32BE',
                'scale_factor': 0.01,
            },
        ],
    },
    {
        'name': 'Huawei SUN2000 PV String Count',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Number of PV strings',
                'value_id': 'VALUE_ID_META',
                'start_address': 30071,
                'value_type': 'U16',
            },
        ],
    },
    {
        'name': 'Huawei SUN2000 PV {variant}',
        'register_type': 'HoldingRegister',
        'variants': ['No Strings', '1 String', '2 Strings', '3 Strings', '4 Strings', '5 Strings', '6 Strings', '7 Strings', '8 Strings', '9 Strings'],
        'values': [
            {
                'name': 'PV1 voltage [0.1 V]',
                'value_id': 'VoltagePV1',
                'start_address': 32016,
                'value_type': 'U16',
                'scale_factor': 0.1,
                'variants': ['1 String', '2 Strings', '3 Strings', '4 Strings', '5 Strings', '6 Strings', '7 Strings', '8 Strings', '9 Strings'],
            },
            {
                'name': 'PV1 current [0.01 A]',
                'value_id': 'CurrentPV1Export',
                'start_address': 32017,
                'value_type': 'S16',
                'scale_factor': 0.01,
                'variants': ['1 String', '2 Strings', '3 Strings', '4 Strings', '5 Strings', '6 Strings', '7 Strings', '8 Strings', '9 Strings'],
            },
            {
                'name': 'PV2 voltage [0.1 V]',
                'value_id': 'VoltagePV2',
                'start_address': 32018,
                'value_type': 'S16',
                'scale_factor': 0.1,
                'variants': ['2 Strings', '3 Strings', '4 Strings', '5 Strings', '6 Strings', '7 Strings', '8 Strings', '9 Strings'],
            },
            {
                'name': 'PV2 current [0.01 A]',
                'value_id': 'CurrentPV2Export',
                'start_address': 32019,
                'value_type': 'S16',
                'scale_factor': 0.01,
                'variants': ['2 Strings', '3 Strings', '4 Strings', '5 Strings', '6 Strings', '7 Strings', '8 Strings', '9 Strings'],
            },
            {
                'name': 'PV3 voltage [0.1 V]',
                'value_id': 'VoltagePV3',
                'start_address': 32020,
                'value_type': 'S16',
                'scale_factor': 0.1,
                'variants': ['3 Strings', '4 Strings', '5 Strings', '6 Strings', '7 Strings', '8 Strings', '9 Strings'],
            },
            {
                'name': 'PV3 current [0.01 A]',
                'value_id': 'CurrentPV3Export',
                'start_address': 32021,
                'value_type': 'S16',
                'scale_factor': 0.01,
                'variants': ['3 Strings', '4 Strings', '5 Strings', '6 Strings', '7 Strings', '8 Strings', '9 Strings'],
            },
            {
                'name': 'PV4 voltage [0.1 V]',
                'value_id': 'VoltagePV4',
                'start_address': 32022,
                'value_type': 'S16',
                'scale_factor': 0.1,
                'variants': ['4 Strings', '5 Strings', '6 Strings', '7 Strings', '8 Strings', '9 Strings'],
            },
            {
                'name': 'PV4 current [0.01 A]',
                'value_id': 'CurrentPV4Export',
                'start_address': 32023,
                'value_type': 'S16',
                'scale_factor': 0.01,
                'variants': ['4 Strings', '5 Strings', '6 Strings', '7 Strings', '8 Strings', '9 Strings'],
            },
            {
                'name': 'PV5 voltage [0.1 V]',
                'value_id': 'VoltagePV5',
                'start_address': 32024,
                'value_type': 'S16',
                'scale_factor': 0.1,
                'variants': ['5 Strings', '6 Strings', '7 Strings', '8 Strings', '9 Strings'],
            },
            {
                'name': 'PV5 current [0.01 A]',
                'value_id': 'CurrentPV5Export',
                'start_address': 32025,
                'value_type': 'S16',
                'scale_factor': 0.01,
                'variants': ['5 Strings', '6 Strings', '7 Strings', '8 Strings', '9 Strings'],
            },
            {
                'name': 'PV6 voltage [0.1 V]',
                'value_id': 'VoltagePV6',
                'start_address': 32026,
                'value_type': 'S16',
                'scale_factor': 0.1,
                'variants': ['6 Strings', '7 Strings', '8 Strings', '9 Strings'],
            },
            {
                'name': 'PV6 current [0.01 A]',
                'value_id': 'CurrentPV6Export',
                'start_address': 32027,
                'value_type': 'S16',
                'scale_factor': 0.01,
                'variants': ['6 Strings', '7 Strings', '8 Strings', '9 Strings'],
            },
            {
                'name': 'PV7 voltage [0.1 V]',
                'value_id': 'VoltagePV7',
                'start_address': 32028,
                'value_type': 'S16',
                'scale_factor': 0.1,
                'variants': ['7 Strings', '8 Strings', '9 Strings'],
            },
            {
                'name': 'PV7 current [0.01 A]',
                'value_id': 'CurrentPV7Export',
                'start_address': 32029,
                'value_type': 'S16',
                'scale_factor': 0.01,
                'variants': ['7 Strings', '8 Strings', '9 Strings'],
            },
            {
                'name': 'PV8 voltage [0.1 V]',
                'value_id': 'VoltagePV8',
                'start_address': 32030,
                'value_type': 'S16',
                'scale_factor': 0.1,
                'variants': ['8 Strings', '9 Strings'],
            },
            {
                'name': 'PV8 current [0.01 A]',
                'value_id': 'CurrentPV8Export',
                'start_address': 32031,
                'value_type': 'S16',
                'scale_factor': 0.01,
                'variants': ['8 Strings', '9 Strings'],
            },
            {
                'name': 'PV9 voltage [0.1 V]',
                'value_id': 'VoltagePV9',
                'start_address': 32032,
                'value_type': 'S16',
                'scale_factor': 0.1,
                'variants': ['9 Strings'],
            },
            {
                'name': 'PV9 current [0.01 A]',
                'value_id': 'CurrentPV9Export',
                'start_address': 32033,
                'value_type': 'S16',
                'scale_factor': 0.01,
                'variants': ['9 Strings'],
            },
            {
                'name': 'Input power [W]',
                'value_id': 'PowerPVSumExport',
                'start_address': 32064,
                'value_type': 'S32BE',
            },
            {
                'name': 'Input power signed [W]',
                'value_id': 'PowerPVSumImExDiff',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
            {
                'name': 'Accumulated energy yield [0.01 kWh]',
                'value_id': 'EnergyPVSumExport',
                'start_address': 32106,
                'value_type': 'U32BE',
                'scale_factor': 0.01,
            },
        ],
    },
]
