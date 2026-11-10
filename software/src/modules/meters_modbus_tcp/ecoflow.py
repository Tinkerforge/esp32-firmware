display_names = [
    ('Ecoflow Powerocean', {
        'en': 'EcoFlow PowerOcean [Plus] / OCEAN 2',
        'de': 'EcoFlow PowerOcean [Plus] / OCEAN 2',
    }),
]

table_prototypes = [
    ('Ecoflow Powerocean', ['device_address', 'virtual_meter']),
]

default_device_addresses = [
    ('Ecoflow Powerocean', 1),
]

specs = [
    {
        'name': 'Ecoflow Powerocean',
        'table_id': 'Ecoflow Powerocean',
        'virtual_meter': 'Inverter',
        'fixed_location': 'Inverter',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Inverter Output Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 0x0225,
                'value_type': 'S32LE',
            },
            {
                'name': 'Inverter Phase A Voltage [V]',
                'value_id': 'VoltageL1N',
                'start_address': 0x0243,
                'value_type': 'F32LE',
            },
            {
                'name': 'Inverter Phase B Voltage [V]',
                'value_id': 'VoltageL2N',
                'start_address': 0x0245,
                'value_type': 'F32LE',
            },
            {
                'name': 'Inverter Phase C Voltage [V]',
                'value_id': 'VoltageL3N',
                'start_address': 0x0247,
                'value_type': 'F32LE',
            },
            {
                'name': 'Inverter Phase A Current [A]',
                'value_id': 'CurrentL1ImExSum',
                'start_address': 0x0249,
                'value_type': 'F32LE',
            },
            {
                'name': 'Inverter Phase B Current [A]',
                'value_id': 'CurrentL2ImExSum',
                'start_address': 0x024B,
                'value_type': 'F32LE',
            },
            {
                'name': 'Inverter Phase C Current [A]',
                'value_id': 'CurrentL3ImExSum',
                'start_address': 0x024D,
                'value_type': 'F32LE',
            },
            {
                'name': 'Inverter Temperature [°C]',
                'value_id': 'Temperature',
                'start_address': 0x024F,
                'value_type': 'F32LE',
            },
            {
                'name': 'Inverter Frequency [Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 0x0251,
                'value_type': 'F32LE',
            },
            {
                'name': 'Inverter AC Energy Input - Accumulated [kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 0x0890,
                'value_type': 'F32LE',
            },
            {
                'name': 'Inverter AC Energy Output - Accumulated [kWh]',
                'value_id': 'EnergyActiveLSumExport',
                'start_address': 0x08A0,
                'value_type': 'F32LE',
            },
        ],
    },
    {
        'name': 'Ecoflow Powerocean Grid',
        'table_id': 'Ecoflow Powerocean',
        'virtual_meter': 'Grid',
        'fixed_location': 'Grid',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Current Grid power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 0x0208,
                'value_type': 'F32LE',
            },
            {
                'name': 'Grid Phase A Current [A]',
                'value_id': 'CurrentL1ImExSum',
                'start_address': 0x022E,
                'value_type': 'F32LE',
            },
            {
                'name': 'Grid Phase B Current [A]',
                'value_id': 'CurrentL2ImExSum',
                'start_address': 0x0230,
                'value_type': 'F32LE',
            },
            {
                'name': 'Grid Phase C Current [A]',
                'value_id': 'CurrentL3ImExSum',
                'start_address': 0x0232,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase A Voltage [V]',
                'value_id': 'VoltageL1N',
                'start_address': 0x0234,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase B Voltage [V]',
                'value_id': 'VoltageL2N',
                'start_address': 0x0236,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase C Voltage [V]',
                'value_id': 'VoltageL3N',
                'start_address': 0x0238,
                'value_type': 'F32LE',
            },
            {
                'name': 'Metered Grid Energy Draw - Accumulated [kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 0x0870,
                'value_type': 'F32LE',
            },
            {
                'name': 'Metered Grid Energy Feed - Accumulated [kWh]',
                'value_id': 'EnergyActiveLSumExport',
                'start_address': 0x0880,
                'value_type': 'F32LE',
            },
        ],
    },
    {
        'name': 'Ecoflow Powerocean Battery',
        'table_id': 'Ecoflow Powerocean',
        'virtual_meter': 'Battery',
        'fixed_location': 'Battery',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Current Total Battery Power [W]',
                'value_id': 'PowerDCChaDisDiff',
                'start_address': 0x020C,
                'value_type': 'F32LE',
            },
            {
                'name': 'System State of Charge [%]',
                'value_id': 'StateOfCharge',
                'start_address': 0x020E,
                'value_type': 'U16',
            },
            {
                'name': 'Battery Capacity [0.001 kWh]',
                'value_id': 'Capacity',
                'start_address': 0x0227,
                'value_type': 'U32LE',
                'scale_factor': 0.001,
            },
            {
                'name': 'Battery Voltage [V]',
                'value_id': 'VoltageDC',
                'start_address': 0x023D,
                'value_type': 'F32LE',
            },
            {
                'name': 'Battery Current [A]',
                'value_id': 'CurrentDCChaDisDiff',
                'start_address': 0x023F,
                'value_type': 'F32LE',
            },
            {
                'name': 'Battery Temperature [°C]',
                'value_id': 'Temperature',
                'start_address': 0x0241,
                'value_type': 'F32LE',
            },
            {
                'name': 'Battery Charge Energy - Accumulated [kWh]',
                'value_id': 'EnergyDCCharge',
                'start_address': 0x08B0,
                'value_type': 'F32LE',
            },
            {
                'name': 'Battery Discharge Energy - Accumulated [kWh]',
                'value_id': 'EnergyDCDischarge',
                'start_address': 0x08C0,
                'value_type': 'F32LE',
            },
        ],
    },
    {
        'name': 'Ecoflow Powerocean Load',
        'table_id': 'Ecoflow Powerocean',
        'virtual_meter': 'Load',
        'fixed_location': 'Load',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Current Total Load Power [W]',
                'value_id': 'PowerActiveLSumImport',
                'start_address': 0x0206,
                'value_type': 'F32LE',
            },
            {
                'name': 'Current Total Load Power - Signed [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 0x0206,
                'value_type': 'F32LE',
                'duplicate': True,
            },
        ],
    },
    {
        'name': 'Ecoflow Powerocean PV',
        'table_id': 'Ecoflow Powerocean',
        'virtual_meter': 'PV',
        'fixed_location': 'PV',
        'register_type': 'HoldingRegister',
        'values': [
            {
                'name': 'Current Total Solar Power [W]',
                'value_id': 'PowerPVSumExport',
                'start_address': 0x020A,
                'value_type': 'F32LE',
            },
            {
                'name': 'PV1 Voltage [V]',
                'value_id': 'VoltagePV1',
                'start_address': 0x0253,
                'value_type': 'F32LE',
            },
            {
                'name': 'PV2 Voltage [V]',
                'value_id': 'VoltagePV2',
                'start_address': 0x0255,
                'value_type': 'F32LE',
            },
            {
                'name': 'PV3 Voltage [V]',
                'value_id': 'VoltagePV3',
                'start_address': 0x0257,
                'value_type': 'F32LE',
            },
            {
                'name': 'PV1 Current [A]',
                'value_id': 'CurrentPV1Export',
                'start_address': 0x0259,
                'value_type': 'F32LE',
            },
            {
                'name': 'PV2 Current [A]',
                'value_id': 'CurrentPV2Export',
                'start_address': 0x025B,
                'value_type': 'F32LE',
            },
            {
                'name': 'PV3 Current [A]',
                'value_id': 'CurrentPV3Export',
                'start_address': 0x025D,
                'value_type': 'F32LE',
            },
            {
                'name': 'Average PV voltage [V]',
                'value_id': 'VoltagePVAvg',
                'calculate': True,
            },
            {
                'name': 'Total PV current [A]',
                'value_id': 'CurrentPVSumExport',
                'calculate': True,
            },
            {
                'name': 'Captured Solar Energy - Accumulated [kWh]',
                'value_id': 'EnergyPVSumExport',
                'start_address': 0x08D0,
                'value_type': 'F32LE',
            },
        ],
    },
]

for spec in specs:
    for value in spec['values']:
        if value.get('calculate', False):
            continue

        value['start_address'] += 40001
