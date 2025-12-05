table_prototypes = [
    ('Sungrow Hybrid Inverter', ['device_address', 'virtual_meter']),
    ('Sungrow String Inverter', ['device_address', 'virtual_meter']),
]

default_device_addresses = [
    ('Sungrow Hybrid Inverter', 1),
    ('Sungrow String Inverter', 1),
]

inverter_values = [
    {
        'name': 'Device Type',
        'value_id': 'VALUE_ID_DEBUG',
        'start_number': 5000,
        'value_type': 'U16',
    },
    {
        'name': 'Nominal Output Power [0.1 kW]',
        'value_id': 'VALUE_ID_DEBUG',
        'start_number': 5001,
        'value_type': 'U16',
    },
    {
        'name': 'Total Output Energy [0.1 kWh]',
        'value_id': 'EnergyActiveLSumExport',
        'start_number': 5004,
        'value_type': 'U32LE',
        'scale_factor': 0.1,
        'variants': ['Hybrid'],
    },
    {
        'name': 'Total Output Energy [kWh]',
        'value_id': 'EnergyActiveLSumExport',
        'start_number': 5004,
        'value_type': 'U32LE',
        'variants': ['String'],
    },
    {
        'name': 'Inside Temperature [0.1 °C]',
        'value_id': 'TemperatureCabinet',
        'start_number': 5008,
        'value_type': 'S16',
        'scale_factor': 0.1,
    },
    {
        'name': 'Total Apparent Power [VA]',  # FIXME: not available for all device types
        'value_id': 'PowerApparentLSumImExDiff',
        'start_number': 5009,
        'value_type': 'U32LE',
        'variants': ['String'],
    },
    {
        'name': 'Phase A Current [0.1 A]',  # FIXME: not available for all device types
        'value_id': 'CurrentL1Export',
        'start_number': 5022,
        'value_type': 'U16',
        'scale_factor': 0.1,
        'variants': ['String'],
    },
    {
        'name': 'Phase B Current [0.1 A]',  # FIXME: not available for all device types
        'value_id': 'CurrentL2Export',
        'start_number': 5023,
        'value_type': 'U16',
        'scale_factor': 0.1,
        'variants': ['String'],
    },
    {
        'name': 'Phase C Current [0.1 A]',  # FIXME: not available for all device types
        'value_id': 'CurrentL3Export',
        'start_number': 5024,
        'value_type': 'U16',
        'scale_factor': 0.1,
        'variants': ['String'],
    },
    {
        'name': 'Total Active Power [W]',
        'value_id': 'PowerActiveLSumExport',
        'start_number': 5031,
        'value_type': 'U32LE',
        'variants': ['String'],
    },
    {
        'name': 'Total Active Power Signed [W]',
        'value_id': 'PowerActiveLSumImExDiff',
        'start_number': 'START_NUMBER_VIRTUAL',
        'variants': ['String'],
    },
    {
        'name': 'Reactive Power [var]',
        'value_id': 'PowerReactiveLSumIndCapDiff',
        'start_number': 5033,
        'value_type': 'S32LE',
    },
    {
        'name': 'Power Factor [0.001]',
        'value_id': 'PowerFactorLSumDirectional',
        'start_number': 5035,
        'value_type': 'S16',
        'scale_factor': 0.001,
    },
    {
        'name': 'Nominal Reactive Power [0.1 kvar]',
        'value_id': 'VALUE_ID_DEBUG',
        'start_number': 5049,
        'value_type': 'U16',
        'variants': ['String'],
    },
    {
        'name': 'Total Direct Energy Consumption [0.1 kWh]',
        'value_id': 'VALUE_ID_DEBUG',
        'start_number': 5103,
        'value_type': 'U32LE',
        'scale_factor': 0.1,
        'variants': ['String'],
    },
    {
        'name': 'Total Output Energy Alternative [0.1 kWh]',  # FIXME: use this instead of 5004, if available
        'value_id': 'VALUE_ID_DEBUG',
        'start_number': 5144,
        'value_type': 'U32LE',
        'scale_factor': 0.1,
        'variants': ['String'],
    },
    {
        'name': 'Total Direct Energy Consumption [0.1 kWh]',
        'value_id': 'VALUE_ID_DEBUG',
        'start_number': 13018,
        'value_type': 'U32LE',
        'scale_factor': 0.1,
        'variants': ['Hybrid'],
    },
    {
        'name': 'Total Active Power [W]',
        'value_id': 'PowerActiveLSumImExDiff',
        'start_number': 13034,
        'value_type': 'S32LE',
        'scale_factor': -1.0,
        'variants': ['Hybrid'],
    },
]

inverter_phase_voltage_values = [
    {
        'name': 'A-N Voltage [0.1 V]',  # FIXME: not available for all string device types
        'value_id': 'VoltageL1N',
        'start_number': 5019,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'B-N Voltage [0.1 V]',  # FIXME: not available for all string device types
        'value_id': 'VoltageL2N',
        'start_number': 5020,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'C-N Voltage [0.1 V]',  # FIXME: not available for all string device types
        'value_id': 'VoltageL3N',
        'start_number': 5021,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
]

inverter_line_voltage_values = [
    {
        'name': 'A-B Voltage [0.1 V]',  # FIXME: not available for all string device types
        'value_id': 'VoltageL1L2',
        'start_number': 5019,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'B-C Voltage [0.1 V]',  # FIXME: not available for all string device types
        'value_id': 'VoltageL2L3',
        'start_number': 5020,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
    {
        'name': 'C-A Voltage [0.1 V]',  # FIXME: not available for all string device types
        'value_id': 'VoltageL3L1',
        'start_number': 5021,
        'value_type': 'U16',
        'scale_factor': 0.1,
    },
]

hybrid_inverter_phase_current_values = [
    {
        'name': 'Phase A Current [0.1 A]',
        'value_id': 'CurrentL1ImExDiff',
        'start_number': 13031,
        'value_type': 'S16',
        'scale_factor': -0.1,  # FIXME: is the current always positive?
        'variants': ['Hybrid'],
    },
    {
        'name': 'Phase B Current [0.1 A]',
        'value_id': 'CurrentL2ImExDiff',
        'start_number': 13032,
        'value_type': 'S16',
        'scale_factor': -0.1,  # FIXME: is the current always positive?
        'variants': ['Hybrid'],
    },
    {
        'name': 'Phase C Current [0.1 A]',
        'value_id': 'CurrentL3ImExDiff',
        'start_number': 13033,
        'value_type': 'S16',
        'scale_factor': -0.1,  # FIXME: is the current always positive?
        'variants': ['Hybrid'],
    },
]

specs = [
    {
        'name': 'Sungrow {variant} Inverter Output Type',
        'variants': ['Hybrid', 'String'],
        'virtual_meter': ('Sungrow {variant} Inverter', 'Inverter'),
        'default_location': 'Inverter',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Output Type',
                'value_id': 'VALUE_ID_META',
                'start_number': 5002,
                'value_type': 'U16',
            },
        ],
    },
    {
        'name': 'Sungrow {variant} Inverter 1P2L',  # output type 1
        'variants': ['Hybrid', 'String'],
        'default_location': 'Inverter',
        'register_type': 'InputRegister',
        'values': inverter_values + inverter_phase_voltage_values[:1] + hybrid_inverter_phase_current_values[:1],
    },
    {
        'name': 'Sungrow {variant} Inverter 3P4L',  # output type 2
        'variants': ['Hybrid', 'String'],
        'default_location': 'Inverter',
        'register_type': 'InputRegister',
        'values': inverter_values + inverter_phase_voltage_values + hybrid_inverter_phase_current_values,
    },
    {
        'name': 'Sungrow {variant} Inverter 3P3L',  # output type 3
        'variants': ['Hybrid', 'String'],
        'default_location': 'Inverter',
        'register_type': 'InputRegister',
        'values': inverter_values + inverter_line_voltage_values + hybrid_inverter_phase_current_values,
    },
    {
        'name': 'Sungrow {variant} Inverter Grid',
        'variants': ['Hybrid', 'String'],
        'virtual_meter': ('Sungrow {variant} Inverter', 'Grid'),
        'default_location': 'Grid',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Grid Frequency [0.1 Hz]',
                'value_id': 'FrequencyLAvg',
                'start_number': 5036,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Grid Frequency Alternative [0.01 Hz]',  # FIXME: use this instead of 5036, if available
                'value_id': 'VALUE_ID_DEBUG',
                'start_number': 5148,
                'value_type': 'U16',
                'scale_factor': 0.01,
                'variants': ['String'],
            },
            {
                'name': 'Meter Power [W]',  # FIXME: not available for all device types
                'value_id': 'PowerActiveLSumImExDiff',
                'start_number': 5083,
                'value_type': 'S32LE',
                'variants': ['String'],
            },
            {
                'name': 'Meter Phase A Power [W]',  # FIXME: not available for all device types
                'value_id': 'PowerActiveL1ImExDiff',
                'start_number': 5085,
                'value_type': 'S32LE',
                'variants': ['String'],
            },
            {
                'name': 'Meter Phase B Power [W]',  # FIXME: not available for all device types
                'value_id': 'PowerActiveL2ImExDiff',
                'start_number': 5087,
                'value_type': 'S32LE',
                'variants': ['String'],
            },
            {
                'name': 'Meter Phase C Power [W]',  # FIXME: not available for all device types
                'value_id': 'PowerActiveL3ImExDiff',
                'start_number': 5089,
                'value_type': 'S32LE',
                'variants': ['String'],
            },
            {
                'name': 'Total Export Energy [0.1 kWh]',  # FIXME: not available for all device types
                'value_id': 'EnergyActiveLSumExport',
                'start_number': 5095,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
                'variants': ['String'],
            },
            {
                'name': 'Total Import Energy [0.1 kWh]',  # FIXME: not available for all device types
                'value_id': 'EnergyActiveLSumImport',
                'start_number': 5099,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
                'variants': ['String'],
            },
            {
                'name': 'Export Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_number': 13010,
                'value_type': 'S32LE',
                'scale_factor': -1.0,
                'variants': ['Hybrid'],
            },
            {
                'name': 'Total Import Energy [0.1 kWh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_number': 13037,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
                'variants': ['Hybrid'],
            },
            {
                'name': 'Total Export Energy [0.1 kWh]',
                'value_id': 'EnergyActiveLSumExport',
                'start_number': 13046,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
                'variants': ['Hybrid'],
            },
        ],
    },
    {
        'name': 'Sungrow Hybrid Inverter Battery',
        'virtual_meter': ('Sungrow Hybrid Inverter', 'Battery'),
        'default_location': 'Battery',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Running State',
                'value_id': 'VALUE_ID_META',
                'start_number': 13001,
                'value_type': 'U16',
            },
            {
                'name': 'Total Battery Charge Energy From PV [0.1 kWh]',
                'value_id': 'VALUE_ID_DEBUG',
                'start_number': 13013,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery Voltage [0.1 V]',
                'value_id': 'VoltageDC',
                'start_number': 13020,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery Current [0.1 A]',
                'value_id': 'CurrentDCChaDisDiff',
                'start_number': 13021,
                'value_type': 'S16',
                'scale_factor': 0.1,  # sign depends on running state
            },
            {
                'name': 'Battery Power [W]',
                'value_id': 'PowerDCChaDisDiff',
                'start_number': 13022,
                'value_type': 'S16',
                'scale_factor': 1.0,  # sign depends on running state
            },
            {
                'name': 'Battery Level [0.1 %]',
                'value_id': 'StateOfCharge',
                'start_number': 13023,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery State Of Health [0.1 %]',
                'value_id': 'VALUE_ID_DEBUG',
                'start_number': 13024,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery Temperature [0.1 °C]',
                'value_id': 'Temperature',
                'start_number': 13025,
                'value_type': 'S16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Total Battery Discharge Energy [0.1 kWh]',
                'value_id': 'EnergyDCDischarge',
                'start_number': 13027,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
            {
                'name': 'Total Charge Energy [0.1 kWh]',
                'value_id': 'EnergyDCCharge',
                'start_number': 13041,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
            },
        ],
    },
    {
        'name': 'Sungrow {variant} Inverter Load',
        'variants': ['Hybrid', 'String'],
        'virtual_meter': ('Sungrow {variant} Inverter', 'Load'),
        'default_location': 'Load',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Load Power [W]',  # FIXME: not available for all device types
                'value_id': 'PowerActiveLSumImExDiff',
                'start_number': 5091,
                'value_type': 'S32LE',
                'variants': ['String'],
            },
            {
                'name': 'Load Power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_number': 13008,
                'value_type': 'S32LE',
                'variants': ['Hybrid'],
            },
        ],
    },
    {
        'name': 'Sungrow {variant} Inverter PV',
        'variants': ['Hybrid', 'String'],
        'virtual_meter': ('Sungrow {variant} Inverter', 'PV'),
        'default_location': 'PV',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'MPPT1 Voltage [0.1 V]',
                'value_id': 'VoltagePV1',
                'start_number': 5011,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'MPPT1 Current [0.1 A]',
                'value_id': 'CurrentPV1Export',
                'start_number': 5012,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'MPPT2 Voltage [0.1 V]',
                'value_id': 'VoltagePV2',
                'start_number': 5013,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'MPPT2 Current [0.1 A]',
                'value_id': 'CurrentPV2Export',
                'start_number': 5014,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            # FIXME: there are up to 16 MPPT on a string inverter, but they cannot be read
            #        blindly as this may produce IllegalDataAddress errors. one way to handle
            #        this would be to define default values for registers that are used in
            #        case a specific error (e.g. IllegalDataAddress) occurs. this also requires
            #        to adapt the block reading logic, as such registers can only be read
            #        individually to be able to detect the error condition. another way of
            #        handling this would be to read the device type register 5000 and keep a
            #        database of how many MPPT certain device types support
            {
                'name': 'Average MPPT Voltage [0.1 V]',
                'value_id': 'VoltagePVAvg',
                'start_number': 'START_NUMBER_VIRTUAL',
            },
            {
                'name': 'Total MPPT Current [0.1 A]',
                'value_id': 'CurrentPVSumExport',
                'start_number': 'START_NUMBER_VIRTUAL',
            },
            {
                'name': 'Total DC Power [W]',
                'value_id': 'PowerPVSumExport',
                'start_number': 5017,
                'value_type': 'U32LE',
            },
            {
                'name': 'Total PV Generation [0.1 kWh]',
                'value_id': 'EnergyPVSumExport',
                'start_number': 13003,
                'value_type': 'U32LE',
                'scale_factor': 0.1,
                'variants': ['Hybrid'],
            },
        ],
    },
]
