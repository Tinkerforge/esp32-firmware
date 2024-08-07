specs = [
    {
        'name': 'Goodwe Hybrid Inverter',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Total Power of Inverter [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 35138,
                'value_type': 'S16',
                'scale_factor': -1.0,
            },
            {
                'name': 'Air Temperature [0.1 °C]',
                'value_id': 'TemperatureAmbient',
                'start_address': 35174,
                'value_type': 'S16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Module Temperature [0.1 °C]',
                'value_id': 'TemperatureCabinet',
                'start_address': 35175,
                'value_type': 'S16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Radiator Temperature [0.1 °C]',
                'value_id': 'TemperatureHeatSink',
                'start_address': 35176,
                'value_type': 'S16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Total PV Energy [0.1 kWh]',
                'value_id': 'EnergyActiveLSumExport',
                'start_address': 35191,
                'value_type': 'U32LE',  # FIXME: endianness?
                'scale_factor': 0.1,
            },
        ],
    },
    {
        'name': 'Goodwe Hybrid Inverter Grid',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'R Phase Grid Voltage [0.1 V]',
                'value_id': 'VoltageL1N',
                'start_address': 35121,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'R Phase Grid Current [0.1 A]',
                'value_id': 'CurrentL1ImExSum',
                'start_address': 35122,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'R Phase Grid Frequency [0.01 Hz]',
                'value_id': 'FrequencyL1',
                'start_address': 35123,
                'value_type': 'U16',
            },
            {
                'name': 'R Phase Grid Power [W]',
                'value_id': 'PowerActiveL1ImExDiff',
                'start_address': 35125,
                'value_type': 'S16',
                'scale_factor': -1.0,
            },
            {
                'name': 'S Phase Grid Voltage [0.1 V]',
                'value_id': 'VoltageL2N',
                'start_address': 35126,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'S Phase Grid Current [0.1 A]',
                'value_id': 'CurrentL2ImExSum',
                'start_address': 35127,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'S Phase Grid Frequency [0.01 Hz]',
                'value_id': 'FrequencyL2',
                'start_address': 35128,
                'value_type': 'U16',
            },
            {
                'name': 'S Phase Grid Power [W]',
                'value_id': 'PowerActiveL2ImExDiff',
                'start_address': 35130,
                'value_type': 'S16',
                'scale_factor': -1.0,
            },
            {
                'name': 'T Phase Grid Voltage [0.1 V]',
                'value_id': 'VoltageL3N',
                'start_address': 35131,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'T Phase Grid Current [0.1 A]',
                'value_id': 'CurrentL3ImExSum',
                'start_address': 35132,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'T Phase Grid Frequency [0.01 Hz]',
                'value_id': 'FrequencyL3',
                'start_address': 35133,
                'value_type': 'U16',
            },
            {
                'name': 'T Phase Grid Power [W]',
                'value_id': 'PowerActiveL3ImExDiff',
                'start_address': 35135,
                'value_type': 'S16',
                'scale_factor': -1.0,
            },
            {
                'name': 'AC Active Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 35140,
                'value_type': 'S16',
                'scale_factor': -1.0,
            },
            {
                'name': 'AC Reactive Power [var]',
                'value_id': 'PowerReactiveLSumIndCapDiff',
                'start_address': 35140,
                'value_type': 'S16',
                'scale_factor': -1.0,
            },
            {
                'name': 'AC Apparent Power [VA]',
                'value_id': 'PowerApparentLSumImExDiff',
                'start_address': 35140,
                'value_type': 'S16',
                'scale_factor': -1.0,
            },
            {
                'name': 'Total Feed Energy to Grid [0.1 kWh]',
                'value_id': 'EnergyActiveLSumExport',
                'start_address': 35195,
                'value_type': 'U32LE',  # FIXME: endianness?
                'scale_factor': 0.1,
            },
            {
                'name': 'Total Draw Energy from Grid [0.1 kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 35200,
                'value_type': 'U32LE',  # FIXME: endianness?
                'scale_factor': 0.1,
            },
        ],
    },
    {
        'name': 'Goodwe Hybrid Inverter Battery',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'First Group Battery Voltage [0.1 V]',
                'value_id': 'VoltageDC',
                'start_address': 35180,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'First Group Battery Current [0.1 A]',
                'value_id': 'CurrentDCChaDisDiff',
                'start_address': 35181,
                'value_type': 'S16',
                'scale_factor': 0.1,  # FIXME: sign?
            },
            {
                'name': 'First Group Battery Power [W]',
                'value_id': 'PowerDCChaDisDiff',
                'start_address': 35183,
                'value_type': 'S16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Charge Energy [0.1 kWh]',
                'value_id': 'EnergyDCCharge',
                'start_address': 35206,
                'value_type': 'U32LE',  # FIXME: endianness?
                'scale_factor': 0.1,
            },
            {
                'name': 'Discharge Energy [0.1 kWh]',
                'value_id': 'EnergyDCDischarge',
                'start_address': 35209,
                'value_type': 'U32LE',  # FIXME: endianness?
                'scale_factor': 0.1,
            },
            {
                'name': 'BMS Pack Temperature [0.1 °C]',
                'value_id': 'Temperature',
                'start_address': 37003,
                'value_type': 'U32LE',  # FIXME: endianness?
                'scale_factor': 0.1,
            },
            {
                'name': 'First Group Battery Capacity [%]',
                'value_id': 'StateOfCharge',
                'start_address': 37007,
                'value_type': 'U16',
            },
        ],
    },
    {
        'name': 'Goodwe Hybrid Inverter Load',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'R Phase Load Power [W]',
                'value_id': 'PowerActiveL1ImExDiff',
                'start_address': 35164,
                'value_type': 'S16',
                'scale_factor': -1.0,
            },
            {
                'name': 'S Phase Load Power [W]',
                'value_id': 'PowerActiveL2ImExDiff',
                'start_address': 35166,
                'value_type': 'S16',
                'scale_factor': -1.0,
            },
            {
                'name': 'T Phase Load Power [W]',
                'value_id': 'PowerActiveL3ImExDiff',
                'start_address': 35168,
                'value_type': 'S16',
                'scale_factor': -1.0,
            },
            {
                'name': 'Total Load Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 35172,
                'value_type': 'S16',
                'scale_factor': -1.0,
            },
            {
                'name': 'Total Energy of Load [0.1 kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 35200,
                'value_type': 'U32LE',  # FIXME: endianness?
                'scale_factor': 0.1,
            },
        ],
    },
]
