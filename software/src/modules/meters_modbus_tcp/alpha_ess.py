# This is the same register table as Hailei

specs = [
    {
        'name': 'Alpha ESS Hybrid Inverter',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Inverter Voltage L1 [0.1 V]',
                'value_id': 'VoltageL1N',
                'start_address': 0x0400,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Inverter Voltage L2 [0.1 V]',
                'value_id': 'VoltageL2N',
                'start_address': 0x0401,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Inverter Voltage L3 [0.1 V]',
                'value_id': 'VoltageL3N',
                'start_address': 0x0402,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Inverter Current L1 [0.1 A]',
                'value_id': 'CurrentL1ImExDiff',
                'start_address': 0x0403,
                'value_type': 'S16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Inverter Current L2 [0.1 A]',
                'value_id': 'CurrentL2ImExDiff',
                'start_address': 0x0404,
                'value_type': 'S16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Inverter Current L3 [0.1 A]',
                'value_id': 'CurrentL3ImExDiff',
                'start_address': 0x0405,
                'value_type': 'S16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Inverter Power L1 [W]',
                'value_id': 'PowerActiveL1ImExDiff',
                'start_address': 0x0406,
                'value_type': 'S32BE',
                'scale_factor': -1.0,
            },
            {
                'name': 'Inverter Power L2 [W]',
                'value_id': 'PowerActiveL2ImExDiff',
                'start_address': 0x0408,
                'value_type': 'S32BE',
                'scale_factor': -1.0,
            },
            {
                'name': 'Inverter Power L3 [W]',
                'value_id': 'PowerActiveL3ImExDiff',
                'start_address': 0x040A,
                'value_type': 'S32BE',
                'scale_factor': -1.0,
            },
            {
                'name': 'Inverter Total Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 0x040C,
                'value_type': 'S32BE',
                'scale_factor': -1.0,
            },
            {
                'name': 'Inverter Grid Frequency [0.01 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 0x041C,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
            {
                'name': 'Inverter Temperature [0.1 °C]',
                'value_id': 'Temperature',
                'start_address': 0x0435,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Inverter Total PV Energy [0.1 kWh]',
                'value_id': 'EnergyDCExport',
                'start_address': 0x043E,
                'value_type': 'U32BE',
                'scale_factor': 0.1,
            },
        ],
    },
    {
        'name': 'Alpha ESS Hybrid Inverter Grid',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Total energy feed to grid [0.01 kWh]',
                'value_id': 'EnergyActiveLSumExport',
                'start_address': 0x0010,
                'value_type': 'U32BE',
                'scale_factor': 0.01,
            },
            {
                'name': 'Total energy consume from grid [0.01 kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 0x0012,
                'value_type': 'U32BE',
                'scale_factor': 0.01,
            },
            {
                'name': 'Voltage of A phase [V]',
                'value_id': 'VoltageL1N',
                'start_address': 0x0014,
                'value_type': 'U16',
            },
            {
                'name': 'Voltage of B phase [V]',
                'value_id': 'VoltageL2N',
                'start_address': 0x0015,
                'value_type': 'U16',
            },
            {
                'name': 'Voltage of C phase [V]',
                'value_id': 'VoltageL3N',
                'start_address': 0x0016,
                'value_type': 'U16',
            },
            {
                'name': 'Current of A phase [0.1 A]',
                'value_id': 'CurrentL1ImExDiff',
                'start_address': 0x0017,
                'value_type': 'S16',
                'scale_factor': 0.1,  # FIXME: sign?
            },
            {
                'name': 'Current of B phase [0.1 A]',
                'value_id': 'CurrentL2ImExDiff',
                'start_address': 0x0018,
                'value_type': 'S16',
                'scale_factor': 0.1,  # FIXME: sign?
            },
            {
                'name': 'Current of C phase [0.1 A]',
                'value_id': 'CurrentL3ImExDiff',
                'start_address': 0x0019,
                'value_type': 'S16',
                'scale_factor': 0.1,  # FIXME: sign?
            },
            {
                'name': 'Frequency [0.01 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 0x001A,
                'value_type': 'U16',
                'scale_factor': 0.01,
            },
            {
                'name': 'Active power of A phase [W]',
                'value_id': 'PowerActiveL1ImExDiff',
                'start_address': 0x001B,
                'value_type': 'S32BE',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Active power of B phase [W]',
                'value_id': 'PowerActiveL2ImExDiff',
                'start_address': 0x001D,
                'value_type': 'S32BE',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Active power of C phase [W]',
                'value_id': 'PowerActiveL3ImExDiff',
                'start_address': 0x001F,
                'value_type': 'S32BE',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Total active power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 0x0021,
                'value_type': 'S32BE',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Reactive power of A phase [var]',
                'value_id': 'PowerReactiveL1IndCapDiff',
                'start_address': 0x0023,
                'value_type': 'S32BE',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Reactive power of B phase [var]',
                'value_id': 'PowerReactiveL2IndCapDiff',
                'start_address': 0x0025,
                'value_type': 'S32BE',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Reactive power of C phase [var]',
                'value_id': 'PowerReactiveL3IndCapDiff',
                'start_address': 0x0027,
                'value_type': 'S32BE',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Total reactive power [var]',
                'value_id': 'PowerReactiveLSumIndCapDiff',
                'start_address': 0x0029,
                'value_type': 'S32BE',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Apparent power of A phase [VA]',
                'value_id': 'PowerApparentL1ImExDiff',
                'start_address': 0x002B,
                'value_type': 'S32BE',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Apparent power of B phase [VA]',
                'value_id': 'PowerApparentL2ImExDiff',
                'start_address': 0x002D,
                'value_type': 'S32BE',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Apparent power of C phase [VA]',
                'value_id': 'PowerApparentL3ImExDiff',
                'start_address': 0x002F,
                'value_type': 'S32BE',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Total apparent power [VA]',
                'value_id': 'PowerApparentLSumImExDiff',
                'start_address': 0x0031,
                'value_type': 'S32BE',
                'scale_factor': 1.0,  # FIXME: sign?
            },
            {
                'name': 'Power factor of A phase [0.01]',
                'value_id': 'PowerFactorL1Directional',
                'start_address': 0x002B,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Power factor of B phase [0.01]',
                'value_id': 'PowerFactorL2Directional',
                'start_address': 0x002D,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Power factor of C phase [0.01]',
                'value_id': 'PowerFactorL3Directional',
                'start_address': 0x002F,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
            {
                'name': 'Total power factor [0.01]',
                'value_id': 'PowerFactorLSumDirectional',
                'start_address': 0x0031,
                'value_type': 'S16',
                'scale_factor': 0.01,  # FIXME: sign?
            },
        ],
    },
    {
        'name': 'Alpha ESS Hybrid Inverter Battery',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Battery voltage [0.1 V]',
                'value_id': 'VoltageDC',
                'start_address': 0x0100,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery current [0.1 A]',
                'value_id': 'CurrentDCChaDisDiff',
                'start_address': 0x0101,
                'value_type': 'S16',
                'scale_factor': -0.1,
            },
            {
                'name': 'Battery SOC [0.1 %]',
                'value_id': 'StateOfCharge',
                'start_address': 0x0102,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Max cell temperature [0.01 °C]',
                'value_id': 'Temperature',
                'start_address': 0x0110,
                'value_type': 'S16',
                'scale_factor': 0.01,
            },
            {
                'name': 'Battery charge energy [0.1 kWh]',
                'value_id': 'EnergyDCCharge',
                'start_address': 0x0120,
                'value_type': 'U32BE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery discharge energy [0.1 kWh]',
                'value_id': 'EnergyDCDischarge',
                'start_address': 0x0122,
                'value_type': 'U32BE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery power [W]',
                'value_id': 'PowerDCChaDisDiff',
                'start_address': 0x0126,
                'value_type': 'S16',
                'scale_factor': -1.0,
            },
        ],
    },
    {
        'name': 'Alpha ESS Hybrid Inverter PV',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'PV1 Voltage [0.1 V]',
                'value_id': 'VoltagePV1',
                'start_address': 0x041D,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'PV1 Current [0.1 A]',
                'value_id': 'CurrentPV1Export',
                'start_address': 0x041E,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'PV1 Power [W]',
                'value_id': 'PowerPV1Export',
                'start_address': 0x041F,
                'value_type': 'U32BE',
            },
            {
                'name': 'PV2 Voltage [0.1 V]',
                'value_id': 'VoltagePV2',
                'start_address': 0x0421,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'PV2 Current [0.1 A]',
                'value_id': 'CurrentPV2Export',
                'start_address': 0x0422,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'PV2 Power [W]',
                'value_id': 'PowerPV2Export',
                'start_address': 0x0423,
                'value_type': 'U32BE',
            },
            {
                'name': 'PV3 Voltage [0.1 V]',
                'value_id': 'VoltagePV3',
                'start_address': 0x0425,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'PV3 Current [0.1 A]',
                'value_id': 'CurrentPV3Export',
                'start_address': 0x0426,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'PV3 Power [W]',
                'value_id': 'PowerPV3Export',
                'start_address': 0x0427,
                'value_type': 'U32BE',
            },
            {
                'name': 'PV4 Voltage [0.1 V]',
                'value_id': 'VoltagePV4',
                'start_address': 0x0429,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'PV4 Current [0.1 A]',
                'value_id': 'CurrentPV4Export',
                'start_address': 0x042A,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'PV4 Power [W]',
                'value_id': 'PowerPV4Export',
                'start_address': 0x042B,
                'value_type': 'U32BE',
            },
            {
                'name': 'PV5 Voltage [0.1 V]',
                'value_id': 'VoltagePV5',
                'start_address': 0x042D,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'PV5 Current [0.1 A]',
                'value_id': 'CurrentPV5Export',
                'start_address': 0x042E,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'PV5 Power [W]',
                'value_id': 'PowerPV5Export',
                'start_address': 0x042F,
                'value_type': 'U32BE',
            },
            {
                'name': 'PV6 Voltage [0.1 V]',
                'value_id': 'VoltagePV6',
                'start_address': 0x0431,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'PV6 Current [0.1 A]',
                'value_id': 'CurrentPV6Export',
                'start_address': 0x0432,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'PV6 Power [W]',
                'value_id': 'PowerPV6Export',
                'start_address': 0x0433,
                'value_type': 'U32BE',
            },
            {
                'name': 'PV Average Voltage [0.1 V]',
                'value_id': 'VoltagePVAvg',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
            {
                'name': 'PV Total Current [0.1 A]',
                'value_id': 'CurrentPVSumExport',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
            {
                'name': 'PV Total Power [W]',
                'value_id': 'PowerPVSumExport',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
            {
                'name': 'PV Total Power Signed [W]',
                'value_id': 'PowerPVSumImExDiff',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
            {
                'name': 'Inverter Total PV Energy [0.1 kWh]',
                'value_id': 'EnergyDCExport',
                'start_address': 0x043E,
                'value_type': 'U32BE',
                'scale_factor': 0.1,
            },
        ],
    },
]
