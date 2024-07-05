specs = [
    {
        'name': 'Victron Energy GX Inverter',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'AC Coupled PV On Output L1 [W]',
                'value_id': 'PowerActiveL1Export',
                'start_address': 808,
                'value_type': 'U16',
            },
            {
                'name': 'AC Coupled PV On Output L2 [W]',
                'value_id': 'PowerActiveL2Export',
                'start_address': 809,
                'value_type': 'U16',
            },
            {
                'name': 'AC Coupled PV On Output L3 [W]',
                'value_id': 'PowerActiveL3Export',
                'start_address': 810,
                'value_type': 'U16',
            },
            {
                'name': 'AC Coupled PV On Output L1+L2+L3 [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
        ],
    },
    {
        'name': 'Victron Energy GX Grid',
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
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 'START_ADDRESS_VIRTUAL',
            },
        ],
    },
]
