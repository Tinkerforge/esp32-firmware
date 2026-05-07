display_names = [
    ('Shelly Pro EM', {
        'en': 'Shelly Pro EM',
        'de': 'Shelly Pro EM',
    }),
    ('Shelly Pro 3EM', {
        'en': 'Shelly Pro 3EM',
        'de': 'Shelly Pro 3EM',
    }),
]

table_prototypes = [
    ('Shelly Pro EM', [
        'device_address',
        ('monophase_channel', 'Config::Enum(ShellyEMMonophaseChannel::None)'),
        ('monophase_mapping', 'Config::Enum(ShellyEMMonophaseMapping::None)'),
    ]),
    ('Shelly Pro 3EM', [
        'device_address',
        ('device_profile', 'Config::Enum(ShellyPro3EMDeviceProfile::Triphase)'),
        ('monophase_channel', 'Config::Enum(ShellyEMMonophaseChannel::None)'),
        ('monophase_mapping', 'Config::Enum(ShellyEMMonophaseMapping::None)'),
    ]),
]

table_lookup_extras = [
    ('Shelly Pro EM', '''
        shelly_pro_em.monophase_channel = ephemeral_table_config->get("monophase_channel")->asEnum<ShellyEMMonophaseChannel>();
        shelly_pro_em.monophase_mapping = ephemeral_table_config->get("monophase_mapping")->asEnum<ShellyEMMonophaseMapping>();

        switch (shelly_pro_em.monophase_channel) {
        case ShellyEMMonophaseChannel::None:
            logger.printfln_meter("No Shelly Pro EM Monophase Channel selected");
            break;

        case ShellyEMMonophaseChannel::First:
            switch (shelly_pro_em.monophase_mapping) {
            case ShellyEMMonophaseMapping::None:
                logger.printfln_meter("No Shelly Pro EM Monophase Mapping selected");
                break;

            case ShellyEMMonophaseMapping::L1:
                table = &shelly_em_monophase_channel_1_as_l1_table;
                break;

            case ShellyEMMonophaseMapping::L2:
                table = &shelly_em_monophase_channel_1_as_l2_table;
                break;

            case ShellyEMMonophaseMapping::L3:
                table = &shelly_em_monophase_channel_1_as_l3_table;
                break;

            default:
                logger.printfln_meter("Unknown Shelly Pro EM Monophase Mapping: %u", static_cast<uint8_t>(shelly_pro_em.monophase_mapping));
                break;
            }

            break;

        case ShellyEMMonophaseChannel::Second:
            switch (shelly_pro_em.monophase_mapping) {
            case ShellyEMMonophaseMapping::None:
                logger.printfln_meter("No Shelly Pro EM Monophase Mapping selected");
                break;

            case ShellyEMMonophaseMapping::L1:
                table = &shelly_em_monophase_channel_2_as_l1_table;
                break;

            case ShellyEMMonophaseMapping::L2:
                table = &shelly_em_monophase_channel_2_as_l2_table;
                break;

            case ShellyEMMonophaseMapping::L3:
                table = &shelly_em_monophase_channel_2_as_l3_table;
                break;

            default:
                logger.printfln_meter("Unknown Shelly Pro EM Monophase Mapping: %u", static_cast<uint8_t>(shelly_pro_em.monophase_mapping));
                break;
            }

            break;

        case ShellyEMMonophaseChannel::Third:
            logger.printfln_meter("Impossible Shelly Pro EM Monophase Channel selected: Third");
            break;

        default:
            logger.printfln_meter("Unknown Shelly Pro EM Monophase Channel: %u", static_cast<uint8_t>(shelly_pro_em.monophase_channel));
            break;
        }'''
    ),
    ('Shelly Pro 3EM', '''
        shelly_pro_3em.device_profile = ephemeral_table_config->get("device_profile")->asEnum<ShellyPro3EMDeviceProfile>();
        shelly_pro_3em.monophase_channel = ephemeral_table_config->get("monophase_channel")->asEnum<ShellyEMMonophaseChannel>();
        shelly_pro_3em.monophase_mapping = ephemeral_table_config->get("monophase_mapping")->asEnum<ShellyEMMonophaseMapping>();

        switch (shelly_pro_3em.device_profile) {
        case ShellyPro3EMDeviceProfile::Triphase:
            table = &shelly_em_triphase_table;
            break;

        case ShellyPro3EMDeviceProfile::Monophase:
            switch (shelly_pro_3em.monophase_channel) {
            case ShellyEMMonophaseChannel::None:
                logger.printfln_meter("No Shelly Pro 3EM Monophase Channel selected");
                break;

            case ShellyEMMonophaseChannel::First:
                switch (shelly_pro_3em.monophase_mapping) {
                case ShellyEMMonophaseMapping::None:
                    logger.printfln_meter("No Shelly Pro 3EM Monophase Mapping selected");
                    break;

                case ShellyEMMonophaseMapping::L1:
                    table = &shelly_em_monophase_channel_1_as_l1_table;
                    break;

                case ShellyEMMonophaseMapping::L2:
                    table = &shelly_em_monophase_channel_1_as_l2_table;
                    break;

                case ShellyEMMonophaseMapping::L3:
                    table = &shelly_em_monophase_channel_1_as_l3_table;
                    break;

                default:
                    logger.printfln_meter("Unknown Shelly Pro 3EM Monophase Mapping: %u", static_cast<uint8_t>(shelly_pro_3em.monophase_mapping));
                    break;
                }

                break;

            case ShellyEMMonophaseChannel::Second:
                switch (shelly_pro_3em.monophase_mapping) {
                case ShellyEMMonophaseMapping::None:
                    logger.printfln_meter("No Shelly Pro 3EM Monophase Mapping selected");
                    break;

                case ShellyEMMonophaseMapping::L1:
                    table = &shelly_em_monophase_channel_2_as_l1_table;
                    break;

                case ShellyEMMonophaseMapping::L2:
                    table = &shelly_em_monophase_channel_2_as_l2_table;
                    break;

                case ShellyEMMonophaseMapping::L3:
                    table = &shelly_em_monophase_channel_2_as_l3_table;
                    break;

                default:
                    logger.printfln_meter("Unknown Shelly Pro 3EM Monophase Mapping: %u", static_cast<uint8_t>(shelly_pro_3em.monophase_mapping));
                    break;
                }

                break;

            case ShellyEMMonophaseChannel::Third:
                switch (shelly_pro_3em.monophase_mapping) {
                case ShellyEMMonophaseMapping::None:
                    logger.printfln_meter("No Shelly Pro 3EM Monophase Mapping selected");
                    break;

                case ShellyEMMonophaseMapping::L1:
                    table = &shelly_em_monophase_channel_3_as_l1_table;
                    break;

                case ShellyEMMonophaseMapping::L2:
                    table = &shelly_em_monophase_channel_3_as_l2_table;
                    break;

                case ShellyEMMonophaseMapping::L3:
                    table = &shelly_em_monophase_channel_3_as_l3_table;
                    break;

                default:
                    logger.printfln_meter("Unknown Shelly Pro 3EM Monophase Mapping: %u", static_cast<uint8_t>(shelly_pro_3em.monophase_mapping));
                    break;
                }

                break;

            default:
                logger.printfln_meter("Unknown Shelly Pro 3EM Monophase Channel: %u", static_cast<uint8_t>(shelly_pro_3em.monophase_channel));
                break;
            }

            break;

        default:
            logger.printfln_meter("Unknown Shelly Pro 3EM Device Profile: %u", static_cast<uint8_t>(shelly_pro_3em.device_profile));
            break;
        }'''
    ),
]

default_device_addresses = [
    ('Shelly Pro EM', 1),
    ('Shelly Pro 3EM', 1),
]

monophase_values = [
    {
        'name': 'Channel 1 voltage [V]',
        'value_id': 'VoltageL1N',
        'start_address': 2003,
        'value_type': 'F32LE',
    },
    {
        'name': 'Channel 1 current [A]',
        'value_id': 'CurrentL1ImExSum',
        'start_address': 2005,
        'value_type': 'F32LE',
    },
    {
        'name': 'Channel 1 active power [W]',
        'value_id': 'PowerActiveL1ImExDiff',
        'start_address': 2007,
        'value_type': 'F32LE',
    },
    {
        'name': 'L1+L2+L3 active power [W]',
        'value_id': 'PowerActiveLSumImExDiff',
        'start_address': 'START_ADDRESS_VIRTUAL',
    },
    {
        'name': 'Channel 1 apparent power [VA]',
        'value_id': 'PowerApparentL1ImExSum',
        'start_address': 2009,
        'value_type': 'F32LE',
    },
    {
        'name': 'Channel 1 power factor',
        'value_id': 'PowerFactorL1',
        'start_address': 2011,
        'value_type': 'F32LE',
    },
    {
        'name': 'Channel 1 frequency [Hz]',
        'value_id': 'FrequencyL1',
        'start_address': 2016,
        'value_type': 'F32LE',
    },
    {
        'name': 'Channel 1 total active energy - perpetual count [Wh]',
        'value_id': 'EnergyActiveL1ImportResettable',
        'start_address': 2310,
        'value_type': 'F32LE',
        'scale_factor': 0.001
    },
    {
        'name': 'L1+L2+L3 total active energy - perpetual count [Wh]',
        'value_id': 'EnergyActiveLSumImportResettable',
        'start_address': 'START_ADDRESS_VIRTUAL',
    },
    {
        'name': 'Channel 1 total active returned energy - perpetual count [Wh]',
        'value_id': 'EnergyActiveL1ExportResettable',
        'start_address': 2312,
        'value_type': 'F32LE',
        'scale_factor': 0.001
    },
    {
        'name': 'L1+L2+L3 total active returned energy - perpetual count [Wh]',
        'value_id': 'EnergyActiveLSumExportResettable',
        'start_address': 'START_ADDRESS_VIRTUAL',
    },
]


def make_monophase_values(channel, phase, start_address_offset):
    values = []

    for value in monophase_values:
        new_value = dict(value)

        new_value['name'] = new_value['name'].replace('Channel 1', channel)
        new_value['value_id'] = new_value['value_id'].replace('L1', phase)

        if new_value['start_address'] != 'START_ADDRESS_VIRTUAL':
            new_value['start_address'] += start_address_offset

        values.append(new_value)

    return values


specs = [
    {
        'name': 'Shelly EM Triphase',
        'fixed_location': 'Unknown',
        'register_type': 'InputRegister',
        'values': [
            {
                'name': 'Neutral current [A]',
                'value_id': 'CurrentNImExSum',
                'start_address': 1007,
                'value_type': 'F32LE',
            },
            {
                'name': 'Total current [A]',
                'value_id': 'CurrentLSumImExSum',
                'start_address': 1011,
                'value_type': 'F32LE',
            },
            {
                'name': 'Total active power [W]',
                'value_id': 'PowerActiveLSumImExDiff',
                'start_address': 1013,
                'value_type': 'F32LE',
            },
            {
                'name': 'Total apparent power [VA]',
                'value_id': 'PowerApparentLSumImExSum',
                'start_address': 1015,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase A voltage [V]',
                'value_id': 'VoltageL1N',
                'start_address': 1020,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase A current [A]',
                'value_id': 'CurrentL1ImExSum',
                'start_address': 1022,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase A active power [W]',
                'value_id': 'PowerActiveL1ImExDiff',
                'start_address': 1024,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase A apparent power [VA]',
                'value_id': 'PowerApparentL1ImExSum',
                'start_address': 1026,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase A power factor',
                'value_id': 'PowerFactorL1',
                'start_address': 1028,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase A frequency [Hz]',
                'value_id': 'FrequencyL1',
                'start_address': 1033,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase B voltage [V]',
                'value_id': 'VoltageL2N',
                'start_address': 1040,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase B current [A]',
                'value_id': 'CurrentL2ImExSum',
                'start_address': 1042,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase B active power [W]',
                'value_id': 'PowerActiveL2ImExDiff',
                'start_address': 1044,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase B apparent power [VA]',
                'value_id': 'PowerApparentL2ImExSum',
                'start_address': 1046,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase B power factor',
                'value_id': 'PowerFactorL2',
                'start_address': 1048,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase B frequency [Hz]',
                'value_id': 'FrequencyL2',
                'start_address': 1053,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase C voltage [V]',
                'value_id': 'VoltageL3N',
                'start_address': 1060,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase C current [A]',
                'value_id': 'CurrentL3ImExSum',
                'start_address': 1062,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase C active power [W]',
                'value_id': 'PowerActiveL3ImExDiff',
                'start_address': 1064,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase C apparent power [VA]',
                'value_id': 'PowerApparentL3ImExSum',
                'start_address': 1066,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase C power factor',
                'value_id': 'PowerFactorL3',
                'start_address': 1068,
                'value_type': 'F32LE',
            },
            {
                'name': 'Phase C frequency [Hz]',
                'value_id': 'FrequencyL3',
                'start_address': 1073,
                'value_type': 'F32LE',
            },
            {
                'name': 'Total active energy accumulated for all phases - perpetual count [Wh]',
                'value_id': 'EnergyActiveLSumImport',
                'start_address': 1162,
                'value_type': 'F32LE',
                'scale_factor': 0.001
            },
            {
                'name': 'Total active returned energy accumulated for all phases - perpetual count [Wh]',
                'value_id': 'EnergyActiveLSumExport',
                'start_address': 1164,
                'value_type': 'F32LE',
                'scale_factor': 0.001
            },
            {
                'name': 'Phase A total active energy - perpetual count [Wh]',
                'value_id': 'EnergyActiveL1Import',
                'start_address': 1182,
                'value_type': 'F32LE',
                'scale_factor': 0.001
            },
            {
                'name': 'Phase A total active returned energy - perpetual count [Wh]',
                'value_id': 'EnergyActiveL1Export',
                'start_address': 1184,
                'value_type': 'F32LE',
                'scale_factor': 0.001
            },
            {
                'name': 'Phase B total active energy - perpetual count [Wh]',
                'value_id': 'EnergyActiveL2Import',
                'start_address': 1202,
                'value_type': 'F32LE',
                'scale_factor': 0.001
            },
            {
                'name': 'Phase B total active returned energy - perpetual count [Wh]',
                'value_id': 'EnergyActiveL2Export',
                'start_address': 1204,
                'value_type': 'F32LE',
                'scale_factor': 0.001
            },
            {
                'name': 'Phase C total active energy - perpetual count [Wh]',
                'value_id': 'EnergyActiveL3Import',
                'start_address': 1222,
                'value_type': 'F32LE',
                'scale_factor': 0.001
            },
            {
                'name': 'Phase C total active returned energy - perpetual count [Wh]',
                'value_id': 'EnergyActiveL3Export',
                'start_address': 1224,
                'value_type': 'F32LE',
                'scale_factor': 0.001
            },
        ],
    },
    {
        'name': 'Shelly EM Monophase Channel 1 As L1',
        'fixed_location': 'Unknown',
        'register_type': 'InputRegister',
        'values': make_monophase_values('Channel 1', 'L1', 0)
    },
    {
        'name': 'Shelly EM Monophase Channel 1 As L2',
        'fixed_location': 'Unknown',
        'register_type': 'InputRegister',
        'values': make_monophase_values('Channel 1', 'L2', 0)
    },
    {
        'name': 'Shelly EM Monophase Channel 1 As L3',
        'fixed_location': 'Unknown',
        'register_type': 'InputRegister',
        'values': make_monophase_values('Channel 1', 'L3', 0)
    },
    {
        'name': 'Shelly EM Monophase Channel 2 As L1',
        'fixed_location': 'Unknown',
        'register_type': 'InputRegister',
        'values': make_monophase_values('Channel 2', 'L1', 20)
    },
    {
        'name': 'Shelly EM Monophase Channel 2 As L2',
        'fixed_location': 'Unknown',
        'register_type': 'InputRegister',
        'values': make_monophase_values('Channel 2', 'L2', 20)
    },
    {
        'name': 'Shelly EM Monophase Channel 2 As L3',
        'fixed_location': 'Unknown',
        'register_type': 'InputRegister',
        'values': make_monophase_values('Channel 2', 'L3', 20)
    },
    {
        'name': 'Shelly EM Monophase Channel 3 As L1',
        'fixed_location': 'Unknown',
        'register_type': 'InputRegister',
        'values': make_monophase_values('Channel 3', 'L1', 40)
    },
    {
        'name': 'Shelly EM Monophase Channel 3 As L2',
        'fixed_location': 'Unknown',
        'register_type': 'InputRegister',
        'values': make_monophase_values('Channel 3', 'L2', 40)
    },
    {
        'name': 'Shelly EM Monophase Channel 3 As L3',
        'fixed_location': 'Unknown',
        'register_type': 'InputRegister',
        'values': make_monophase_values('Channel 3', 'L3', 40)
    },
]
