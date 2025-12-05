table_prototypes = [
    ('Victron Energy GX', ['device_address', 'virtual_meter']),
]

default_device_addresses = [
    ('Victron Energy GX', 100),
]

specs = [
    {
        'name': 'Victron Energy GX Grid',
        'virtual_meter': ('Victron Energy GX', 'Grid'),
        'default_location': 'Grid',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Grid L1 [W]',
                'value_id': 'PowerActiveL1ImExDiff',
                'start_address': 820,
                'value_type': 'S16',
            },
            {
                'name': 'Grid L2 [W]',
                'value_id': 'PowerActiveL2ImExDiff',
                'start_address': 821,
                'value_type': 'S16',
            },
            {
                'name': 'Grid L3 [W]',
                'value_id': 'PowerActiveL3ImExDiff',
                'start_address': 822,
                'value_type': 'S16',
            },
        ],
    },
    {
        'name': 'Victron Energy GX Battery',
        'virtual_meter': ('Victron Energy GX', 'Battery'),
        'default_location': 'Battery',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Battery Voltage [0.1 V]',
                'value_id': 'VoltageDC',
                'start_address': 840,
                'value_type': 'U16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery Current [0.1 A]',
                'value_id': 'CurrentDCChaDisDiff',
                'start_address': 841,
                'value_type': 'S16',
                'scale_factor': 0.1,
            },
            {
                'name': 'Battery Power [W]',
                'value_id': 'PowerDCChaDisDiff',
                'start_address': 842,
                'value_type': 'S16',
            },
            {
                'name': 'Battery State Of Charge [%]',
                'value_id': 'StateOfCharge',
                'start_address': 843,
                'value_type': 'U16',
            },
        ],
    },
    {
        'name': 'Victron Energy GX Load',
        'virtual_meter': ('Victron Energy GX', 'Load'),
        'default_location': 'Load',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'AC Consumption L1 [W]',
                'value_id': 'PowerActiveL1Import',
                'start_address': 817,
                'value_type': 'U16',
            },
            {
                'name': 'AC Consumption L2 [W]',
                'value_id': 'PowerActiveL2Import',
                'start_address': 818,
                'value_type': 'U16',
            },
            {
                'name': 'AC Consumption L3 [W]',
                'value_id': 'PowerActiveL3Import',
                'start_address': 819,
                'value_type': 'U16',
            },
            {
                'name': 'AC Consumption L1+L2+L3 [W]',
                'value_id': 'PowerActiveLSumImport',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
            {
                'name': 'AC Consumption L1+L2+L3 Signed [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
        ],
    },
    {
        'name': 'Victron Energy GX PV',
        'virtual_meter': ('Victron Energy GX', 'PV'),
        'default_location': 'PV',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'AC Coupled PV Power On Output L1 [W]',
                'value_id': 'VALUE_ID_META',
                'start_address': 808,
                'value_type': 'U16',
            },
            {
                'name': 'AC Coupled PV Power On Output L2 [W]',
                'value_id': 'VALUE_ID_META',
                'start_address': 809,
                'value_type': 'U16',
            },
            {
                'name': 'AC Coupled PV Power On Output L3 [W]',
                'value_id': 'VALUE_ID_META',
                'start_address': 810,
                'value_type': 'U16',
            },
            {
                'name': 'AC Coupled PV Power On Input L1 [W]',
                'value_id': 'VALUE_ID_META',
                'start_address': 811,
                'value_type': 'U16',
            },
            {
                'name': 'AC Coupled PV Power On Input L2 [W]',
                'value_id': 'VALUE_ID_META',
                'start_address': 812,
                'value_type': 'U16',
            },
            {
                'name': 'AC Coupled PV Power On Input L3 [W]',
                'value_id': 'VALUE_ID_META',
                'start_address': 813,
                'value_type': 'U16',
            },
            {
                'name': 'AC Coupled PV Power On Generator L1 [W]',
                'value_id': 'VALUE_ID_META',
                'start_address': 814,
                'value_type': 'U16',
            },
            {
                'name': 'AC Coupled PV Power On Generator L2 [W]',
                'value_id': 'VALUE_ID_META',
                'start_address': 815,
                'value_type': 'U16',
            },
            {
                'name': 'AC Coupled PV Power On Generator L3 [W]',
                'value_id': 'VALUE_ID_META',
                'start_address': 816,
                'value_type': 'U16',
            },
            {
                'name': 'DC Coupled PV Power [W]',
                'value_id': 'VALUE_ID_META',
                'start_address': 850,
                'value_type': 'U16',
            },
            {
                'name': 'Total PV Power [W]',
                'value_id': 'PowerPVSumExport',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
        ],
    },
]
