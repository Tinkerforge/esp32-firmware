inverter_values = [
    {
        'name': 'Device Type',
        'value_id': 'VALUE_ID_DEBUG',
        'start_address': 5000,
        'value_type': 'U16',
    },
    {
        'name': 'Nominal Output Power [0.1 kW]',
        'value_id': 'VALUE_ID_DEBUG',
        'start_address': 5001,
        'value_type': 'U16',
    },
    {
        'name': 'Total Output Energy [0.1 kWh]',
        'value_id': 'EnergyActiveLSumExport',
        'start_address': 5004,
        'value_type': 'U32LE',
        'scale_factor': 0.1,
        'variant': 'Hybrid',
    },
    {
        'name': 'Total Output Energy [kWh]',
        'value_id': 'EnergyActiveLSumExport',
        'start_address': 5004,
        'value_type': 'U32LE',
        'variant': 'String',
    },
    {
        'name': 'Total Running Time [h]',
        'value_id': 'VALUE_ID_DEBUG',
        'start_address': 5006,
        'value_type': 'U32LE',
        'variant': 'String',
    },
    {
        'name': 'Inside Temperature [0.1 °C]',
        'value_id': 'TemperatureCabinet',
        'start_address': 5008,
        'value_type': 'S16',
        'scale_factor': 0.1,
    },
    {
        'name': 'Total Apparent Power [VA]',  # FIXME: not available for all device types
        'value_id': 'PowerApparentLSumImExDiff',
        'start_address': 5009,
        'value_type': 'U32LE',
        'variant': 'String',
    },
    {
        'name': 'MPPT 1 Voltage [0.1 V]',
        'value_id': 'VALUE_ID_DEBUG',
        'start_address': 5011,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'MPPT 1 Current [0.1 A]',
        'value_id': 'VALUE_ID_DEBUG',
        'start_address': 5012,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'MPPT 2 Voltage [0.1 V]',
        'value_id': 'VALUE_ID_DEBUG',
        'start_address': 5013,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'MPPT 2 Current [0.1 A]',
        'value_id': 'VALUE_ID_DEBUG',
        'start_address': 5014,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'MPPT 3 Voltage [0.1 V]',
        'value_id': 'VALUE_ID_DEBUG',
        'start_address': 5015,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'MPPT 3 Current [0.1 A]',
        'value_id': 'VALUE_ID_DEBUG',
        'start_address': 5016,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'Total DC Power [W]',
        'value_id': 'PowerDCExport',
        'start_address': 5017,
        'value_type': 'U32LE',
    },
    {
        'name': 'Phase A Current [0.1 A]',  # FIXME: not available for all device types
        'value_id': 'CurrentL1Export',
        'start_address': 5022,
        'value_type': 'U16',
        'scale_factor': 0.1,
        'variant': 'String',
    },
    {
        'name': 'Phase B Current [0.1 A]',  # FIXME: not available for all device types
        'value_id': 'CurrentL2Export',
        'start_address': 5023,
        'value_type': 'U16',
        'scale_factor': 0.1,
        'variant': 'String',
    },
    {
        'name': 'Phase C Current [0.1 A]',  # FIXME: not available for all device types
        'value_id': 'CurrentL3Export',
        'start_address': 5024,
        'value_type': 'U16',
        'scale_factor': 0.1,
        'variant': 'String',
    },
    {
        'name': 'Total Active Power Export [W]',
        'value_id': 'PowerActiveLSumExport',
        'start_address': 5031,
        'value_type': 'U32LE',
        'variant': 'String',
    },
    {
        'name': 'Total Active Power [W]',
        'value_id': 'PowerActiveLSumImExDiff',
        'start_address': 'START_ADDRESS_VIRTUAL',
        'variant': 'String',
    },
    {
        'name': 'Reactive Power [var]',
        'value_id': 'PowerReactiveLSumIndCapDiff',
        'start_address': 5033,
        'value_type': 'S32LE',
    },
    {
        'name': 'Power Factor [0.001]',
        'value_id': 'PowerFactorLSumDirectional',
        'start_address': 5035,
        'value_type': 'S16',
        'scale_factor': 0.001,
    },
    {
        'name': 'Nominal Reactive Power [0.1 kvar]',
        'value_id': 'VALUE_ID_DEBUG',
        'start_address': 5049,
        'value_type': 'U16',
        'variant': 'String',
    },
    {
        'name': 'Total Direct Energy Consumption [0.1 kWh]',
        'value_id': 'VALUE_ID_DEBUG',
        'start_address': 5103,
        'value_type': 'U32LE',
        'scale_factor': 0.1,
        'variant': 'String',
    },
    {
        'name': 'Total Output Energy [0.1 kWh]',
        'value_id': 'VALUE_ID_DEBUG',
        'start_address': 5144,  # FIXME: use this instead of 5004, if available
        'value_type': 'U32LE',
        'scale_factor': 0.1,
        'variant': 'String',
    },
    {
        'name': 'Total PV Generation [0.1 kWh]',
        'value_id': 'VALUE_ID_DEBUG',
        'start_address': 13003,
        'value_type': 'U32LE',
        'scale_factor': 0.1,
        'variant': 'Hybrid',
    },
    {
        'name': 'Total Direct Energy Consumption [0.1 kWh]',
        'value_id': 'VALUE_ID_DEBUG',
        'start_address': 13018,
        'value_type': 'U32LE',
        'scale_factor': 0.1,
        'variant': 'Hybrid',
    },
    {
        'name': 'Total Active Power [W]',
        'value_id': 'PowerActiveLSumImExDiff',
        'start_address': 13034,
        'value_type': 'S32LE',
        'scale_factor': -1.0,
        'variant': 'Hybrid',
    },
]

inverter_phase_voltage_values = [
    {
        'name': 'A-N Voltage [0.1 V]',  # FIXME: not available for all string device types
        'value_id': 'VoltageL1N',
        'start_address': 5019,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'B-N Voltage [0.1 V]',  # FIXME: not available for all string device types
        'value_id': 'VoltageL2N',
        'start_address': 5020,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'C-N Voltage [0.1 V]',  # FIXME: not available for all string device types
        'value_id': 'VoltageL3N',
        'start_address': 5021,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
]

inverter_line_voltage_values = [
    {
        'name': 'A-B Voltage [0.1 V]',  # FIXME: not available for all string device types
        'value_id': 'VoltageL1L2',
        'start_address': 5019,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'B-C Voltage [0.1 V]',  # FIXME: not available for all string device types
        'value_id': 'VoltageL2L3',
        'start_address': 5020,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'C-A Voltage [0.1 V]',  # FIXME: not available for all string device types
        'value_id': 'VoltageL3L1',
        'start_address': 5021,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
]

hybrid_inverter_phase_current_values = [
    {
        'name': 'Phase A Current [0.1 A]',
        'value_id': 'CurrentL1ImExDiff',
        'start_address': 13031,
        'value_type': 'S16',
        'scale_factor': -0.1,  # FIXME: is the current always positive?
        'variant': 'Hybrid',
    },
    {
        'name': 'Phase B Current [0.1 A]',
        'value_id': 'CurrentL2ImExDiff',
        'start_address': 13032,
        'value_type': 'S16',
        'scale_factor': -0.1,  # FIXME: is the current always positive?
        'variant': 'Hybrid',
    },
    {
        'name': 'Phase C Current [0.1 A]',
        'value_id': 'CurrentL3ImExDiff',
        'start_address': 13033,
        'value_type': 'S16',
        'scale_factor': -0.1,  # FIXME: is the current always positive?
        'variant': 'Hybrid',
    },
]

specs = [
    {
        'name': 'Sungrow {variant} Inverter Output Type',
        'variants': ['Hybrid', 'String'],
        'register_type': 'InputRegister',
        'start_address_offset': 1,
        'values': [
            {
                'name': 'Output Type',
                'value_id': 'VALUE_ID_META',
                'start_address': 5002,
                'value_type': 'U16',
            },
        ],
    },
    {
        'name': 'Sungrow {variant} Inverter 1P2L',  # output type 1
        'variants': ['Hybrid', 'String'],
        'register_type': 'InputRegister',
        'start_address_offset': 1,
        'values': inverter_values + inverter_phase_voltage_values[:1] + hybrid_inverter_phase_current_values[:1],
    },
    {
        'name': 'Sungrow {variant} Inverter 3P4L',  # output type 2
        'variants': ['Hybrid', 'String'],
        'register_type': 'InputRegister',
        'start_address_offset': 1,
        'values': inverter_values + inverter_phase_voltage_values + hybrid_inverter_phase_current_values,
    },
    {
        'name': 'Sungrow {variant} Inverter 3P3L',  # output type 3
        'variants': ['Hybrid', 'String'],
        'register_type': 'InputRegister',
        'start_address_offset': 1,
        'values': inverter_values + inverter_line_voltage_values + hybrid_inverter_phase_current_values,
    },
    {
        'name': 'Sungrow {variant} Inverter Grid',
        'variants': ['Hybrid', 'String'],
        'register_type': 'InputRegister',
        'start_address_offset': 1,
        'values': [
            {
                'name': 'Grid Frequency [0.1 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_address': 5036,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Grid Frequency [0.01 Hz]',  # FIXME: use this instead of 5036, if available
                'value_id': 'VALUE_ID_DEBUG',
                'start_address': 5148,
                'value_type': 'U16',
                'scale_factor': 0.01,
                'variant': 'String',
            },
            {
                'name': 'Meter Power [W]',  # FIXME: not available for all device types
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 5083,
                'value_type': 'S32LE',
                'scale_factor': -1.0,
                'variant': 'String',
            },
            {
                'name': 'Meter Phase A Power [W]',  # FIXME: not available for all device types
                'value_id': 'PowerActiveL1ImExDiff',
                'start_address': 5085,
                'value_type': 'S32LE',
                'scale_factor': -1.0,
                'variant': 'String',
            },
            {
                'name': 'Meter Phase B Power [W]',  # FIXME: not available for all device types
                'value_id': 'PowerActiveL2ImExDiff',
                'start_address': 5087,
                'value_type': 'S32LE',
                'scale_factor': -1.0,
                'variant': 'String',
            },
            {
                'name': 'Meter Phase C Power [W]',  # FIXME: not available for all device types
                'value_id': 'PowerActiveL3ImExDiff',
                'start_address': 5089,
                'value_type': 'S32LE',
                'scale_factor': -1.0,
                'variant': 'String',
            },
            {
                'name': 'Total Export Energy [0.1 kWh]',  # FIXME: not available for all device types
                'value_id': 'EnergyActiveLSumExport',
                'start_address': 5095,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
                'variant': 'String',
            },
            {
                'name': 'Total Import Energy [0.1 kWh]',  # FIXME: not available for all device types
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 5099,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
                'variant': 'String',
            },
            {
                'name': 'Export Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 13010,
                'value_type': 'S32LE',
                'scale_factor': -1.0,
                'variant': 'Hybrid',
            },
            {
                'name': 'Total Import Energy [0.1 kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 13037,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
                'variant': 'Hybrid',
            },
            {
                'name': 'Total Export Energy [0.1 kWh]',
                'value_id': 'EnergyActiveLSumExport',
                'start_address': 13046,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
                'variant': 'Hybrid',
            },
        ],
    },
    {
        'name': 'Sungrow Hybrid Inverter Battery',
        'register_type': 'InputRegister',
        'start_address_offset': 1,
        'values': [
            {
                'name': 'Running State',
                'value_id': 'VALUE_ID_META',
                'start_address': 13001,
                'value_type': 'U16',
            },
            {
                'name': 'Total Battery Charge Energy From PV [0.1 kWh]',
                'value_id': 'VALUE_ID_DEBUG',
                'start_address': 13013,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery Voltage [0.1 V]',
                'value_id': 'VoltageDC',
                'start_address': 13020,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery Current [0.1 A]',
                'value_id': 'CurrentDCChaDisDiff',
                'start_address': 13021,
                'value_type': 'U16',
                'scale_factor': 0.1,  # sign depends on running state
            },
            {
                'name': 'Battery Power [W]',
                'value_id': 'PowerDCChaDisDiff',
                'start_address': 13022,
                'value_type': 'U16',
                'scale_factor': 1.0,  # sign depends on running state
            },
            {
                'name': 'Battery Level [0.1 %]',
                'value_id': 'StateOfCharge',
                'start_address': 13023,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery State Of Health [0.1 %]',
                'value_id': 'VALUE_ID_DEBUG',
                'start_address': 13024,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery Temperature [0.1 °C]',
                'value_id': 'Temperature',
                'start_address': 13025,
                'value_type': 'S16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Total Battery Discharge Energy [0.1 kWh]',
                'value_id': 'EnergyActiveLSumExport',
                'start_address': 13027,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Total Charge Energy [0.1 kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 13041,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
        ],
    },
    {
        'name': 'Sungrow {variant} Inverter Load',
        'variants': ['Hybrid', 'String'],
        'register_type': 'InputRegister',
        'start_address_offset': 1,
        'values': [
            {
                'name': 'Load Power [W]',  # FIXME: not available for all device types
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 5091,
                'value_type': 'S32LE',
                'variant': 'String',
            },
            {
                'name': 'Load Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 13008,
                'value_type': 'S32LE',
                'variant': 'Hybrid',
            },
        ],
    },
]
