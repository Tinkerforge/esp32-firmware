# Fronius GEN24 Plus abuses MPPT 3 and 4 of SunSpec model 160 to report battery information

specs = [
    {
        'name': 'Fronius GEN24 Plus Hybrid Inverter Battery',
        'register_type': 'HoldingRegister',
        'start_address_offset': 1,
        'values': [
            {
                'name': 'DCA_SF: Current Scale Factor',
                'value_id': 'VALUE_ID_META',
                'start_address': 40256,
                'value_type': 'S16',  # SunSpec: sunssf
            },
            {
                'name': 'DCV_SF: Voltage Scale Factor',
                'value_id': 'VALUE_ID_META',
                'start_address': 40257,
                'value_type': 'S16',  # SunSpec: sunssf
            },
            {
                'name': 'DCW_SF: Power Scale Factor',
                'value_id': 'VALUE_ID_META',
                'start_address': 40258,
                'value_type': 'S16',  # SunSpec: sunssf
            },
            {
                'name': 'DCWH_SF: Energy Scale Factor',
                'value_id': 'VALUE_ID_META',
                'start_address': 40259,
                'value_type': 'S16',  # SunSpec: sunssf
            },
            {
                'name': 'module/3/DCA: DC Charge Current [A]',
                'value_id': 'VALUE_ID_META',
                'start_address': 40313,
                'value_type': 'U16',  # SunSpec: uint16
            },
            {
                'name': 'module/3/DCV: DC Charge Voltage [V]',
                'value_id': 'VALUE_ID_META',
                'start_address': 40314,
                'value_type': 'U16',  # SunSpec: uint16
            },
            {
                'name': 'module/3/DCW: DC Charge Power [W]',
                'value_id': 'VALUE_ID_META',
                'start_address': 40315,
                'value_type': 'U16',  # SunSpec: uint16
            },
            {
                'name': 'module/3/DCWH: Lifetime Charge Energy [Wh]',
                'value_id': 'VALUE_ID_META',
                'start_address': 40316,
                'value_type': 'U32BE',  # SunSpec: acc32
            },
            {
                'name': 'module/4/DCA: DC Discharge Current [A]',
                'value_id': 'VALUE_ID_META',
                'start_address': 40333,
                'value_type': 'U16',  # SunSpec: uint16
            },
            {
                'name': 'module/4/DCV: DC Discharge Voltage [V]',
                'value_id': 'VALUE_ID_META',
                'start_address': 40334,
                'value_type': 'U16',  # SunSpec: uint16
            },
            {
                'name': 'module/4/DCW: DC Discharge Power [W]',
                'value_id': 'VALUE_ID_META',
                'start_address': 40335,
                'value_type': 'U16',  # SunSpec: uint16
            },
            {
                'name': 'module/4/DCWH: Lifetime Discharge Energy [Wh]',
                'value_id': 'VALUE_ID_META',
                'start_address': 40336,
                'value_type': 'U32BE',  # SunSpec: acc32
            },
            {
                'name': 'ChaState: Currently available energy as a percent of the capacity rating [%]',
                'value_id': 'VALUE_ID_META',
                'start_address': 40352,
                'value_type': 'U16',  # SunSpec: uint16
            },
            {
                'name': 'ChaState_SF: Scale factor for available energy percent',
                'value_id': 'VALUE_ID_META',
                'start_address': 40366,
                'value_type': 'S16',  # SunSpec: sunssf
            },
            {
                'name': 'Battery Current [A]',
                'value_id': 'CurrentDCChaDisDiff',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
            {
                'name': 'Battery Voltage [V]',
                'value_id': 'VoltageDC',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
            {
                'name': 'Battery Power [W]',
                'value_id': 'PowerDCChaDisDiff',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
            {
                'name': 'Battery State Of Charge [%]',
                'value_id': 'StateOfCharge',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
            {
                'name': 'Battery Charge Energy [kWh]',
                'value_id': 'EnergyDCCharge',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
            {
                'name': 'Battery Discharge Energy [kWh]',
                'value_id': 'EnergyDCDischarge',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
        ],
    },
]
