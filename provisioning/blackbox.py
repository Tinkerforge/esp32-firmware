import socket
from collections import namedtuple

METREL_HOST = '10.4.2.3'
METREL_PORT = 5335

# maps Metrel result IDs to names
# data available in Metrel "DataDisplay Framework Visualizer"

METREL_PARAMETERS = {
    '4': 'Uiso_INST_EE',
    '11': 'Type_Riso',
    '14': 'RCD_Standard',
    '15': 'I_dN',
    '17': 'RCD_Selectivity',
    '20': 'TYPE_RCD',
    '21': 'Phase',
    '28': 'Fuse_Ir',
    '29': 'Fuse_t',
    '30': 'Z_factor',
    '31': 'Isc_factor',
    '56': 'Type_cont_out',
    '108': 'FuseType',
    '131': 'RCD_use',
    '152': 'Bonding_type',
    '186': 'Rcal_Rlow_LN_LPE_NPE',
    '230': 'Voltage_system',
    '231': 'Voltage_limit_type',
    '232': 'Nominal_voltage',
    '233': 'ZLoop_Test',
    '236': 'I_test',
    '237': 'Protection_fuse_rcd',
    '238': 'Other_RCD_Type',
    '239': 'IdN_IdnDC',
    '240': 'Other_RCD_Test',
    '242': 'Earthing_System',
    '249': 'Comment_1',
    '250': 'Comment_2',
    '257': 'Phase_Z_Auto',
    '338': 'RLow_Current_type',
    '434': 'Voltage_Test',
    '436': 'Test_time_Voltage_INST',
}

METREL_LIMITS = {
    '1': 'Limit_Riso',
    '3': 'Limit_Rlow',
    '6': 'UC_lim',
    '13': 'Fuse_Ia',
    '16': 'Limit_VVoltage_Drop',
    '99': 'MI3155_Uln_LoLim_Percent',
    '100': 'MI3155_Uln_HiLim_Percent',
    '101': 'MI3155_Ulpe_LoLim',
    '102': 'MI3155_Ulpe_HiLim',
    '103': 'MI3155_Unpe_LoLim',
    '104': 'MI3155_Unpe_HiLim',
}

METREL_RESULTS = {
    '1': 'Uin',
    '2': 'Ulpe',
    '3': 'Unpe',
    '4': 'FreqV',
    '9': 'Riso',
    '10': 'Um',
    '11': 'RIn',
    '21': 'R_cont',
    '22': 'R_Plus',
    '23': 'R_Minus',
    '26': 'Cal_R',
    '29': 't_Id',
    '31': 'Id',
    '32': 'Uc_Id',
    '114': 'Uc',
    '261': 'ZPLE_Auto',
    '269': 'ZLN_Auto',
    '270': 'IPSC_LN_Auto',
    '271': 'IPSC_LPE_Auto',
}

CallResult = namedtuple('CallResult', 'success response')
DeviceInfo = namedtuple('DeviceInfo', 'valid product_name model_name firmware_version serial_number calibration_date')
TestResult = namedtuple('TestResult', 'passed parameters limits results')


def debug(*args, **kwags):
    if False:
        print(*args, **kwags)


def bb_call(request):
    with socket.create_connection((METREL_HOST, METREL_PORT)) as s:
        s.settimeout(30)

        debug(f'REQUEST: {request}')

        request_bytes = request.encode('ascii') + b'\r'
        debug(f'SEND: {request_bytes}')

        s.sendall(request_bytes)

        pending_response_bytes = b''
        result_response = []

        while True:
            try:
                data = s.recv(4096)
            except socket.timeout:
                return CallResult(False, 'socket timeout')

            if len(data) == 0:
                return CallResult(False, 'connection closed by peer')

            debug(f'RECV: {data}')

            pending_response_bytes += data

            while b'\r' in pending_response_bytes:
                response_bytes, pending_response_bytes = pending_response_bytes.split(b'\r', 1)
                response = response_bytes.decode('ascii').strip()

                if len(response) == 0:
                    continue

                debug(f'RESPONSE: {response}')

                if response.startswith('BB; ERROR '):
                    return CallResult(False, response.removeprefix('BB; ERROR '))

                if response == 'BB; DONE':
                    return CallResult(True, result_response)

                if request == 'BB; STATUS' and response.startswith('BB; STATUS; '):
                    return CallResult(True, response.removeprefix('BB; STATUS; '))

                if request == 'MWH' and response == 'SER':
                    return CallResult(True, result_response)

                if request.startswith('BB; START_SINGLETEST ') and response == 'BB; ST; END':
                    return CallResult(True, result_response)

                result_response.append(response)


def bb_get_status():
    return bb_call('BB; STATUS')


def bb_enable():
    return bb_call('BB; ENABLE = 1')


def bb_disable():
    return bb_call('BB; ENABLE = 0')


def bb_reset():
    return bb_call('BB; RESET')


def bb_get_device_info():
    result = bb_call('MWH')

    valid = False
    product_name = None
    model_name = None
    firmware_version = None
    serial_number = None
    calibration_date = None

    if not result.success:
        debug(f'MWH failed: {result.response}')
    else:
        valid = True

        for response in result.response:
            key, value = response.split('.', 1)

            if key == '02':
                product_name = value
            elif key == '03':
                model_name = value
            elif key == '04':
                firmware_version = value
            elif key == '07':
                serial_number = value
            elif key == '11':
                calibration_date = value

    return DeviceInfo(valid, product_name, model_name, firmware_version, serial_number, calibration_date)


def bb_start_test(suffix):
    result = bb_call('BB; START_SINGLETEST ' + suffix)

    if not result.success:
        debug(f'ST failed: {result.response}')
        return None

    passed = None
    parameters = {}
    limits = {}
    results = {}

    for response in result.response:
        kind, suffix = response.removeprefix('BB; ST; ').split(' ', 1)

        if kind == 'START':
            pass
        elif kind == 'STATUS':
            if suffix == '= pass':
                passed = True
            elif suffix == '= fail':
                passed = False
            else:
                debug(f'ST response malformed: {result.response}')
                return None
        else:
            if '=' in suffix:
                key, value = suffix.split(' = ', 1)
                value = value.split(';')[0]  # FIXME: drop tail
            else:
                key = suffix
                value = None

            if kind == 'PARAMETER':
                parameters[METREL_PARAMETERS[key]] = value
            elif kind == 'LIMIT':
                limits[METREL_LIMITS[key]] = value
            elif kind == 'RESULT':
                results[METREL_RESULTS[key]] = value
            else:
                debug(f'ST response malformed: {result.response}')
                return None

    return TestResult(passed, parameters, limits, results)


def bb_measure_voltage():
    return bb_start_test('2; P230 = 1-phase; P434 = -; P231 = %; P232 = 230 V; P435 = -; P242 = TN/TT; P436 = 3 s')


def bb_measure_zauto():
    return bb_start_test('183; P15 = 30 mA; P17 = G; P20 = A; P28 = 6 A; P29 = 0.035 s; P31 = 1; P108 = C; P236 = Low; P257 = -')


def bb_measure_rcdi(polarity):
    return bb_start_test(f'13; P131 = other; P238 = MI RCD; P239 = 30 mA / 6 mA d.c.; P441 = d.c.; P242 = TN/TT; P14 = VDE 0664; P166 = -; P21 = ({polarity}); P240 = d.c.')


def bb_measure_riso(type_):
    return bb_start_test(f'4; P4 = 500 V; P11 = {type_}; L1 = 1 MOhm')


def bb_measure_rlow():
    return bb_start_test('7; P56 = LPE; P152 = Rpe; P186 = LN; P338 = standard')


def main():
    status = bb_get_status()

    if not status.success:
        print(status)
        return

    if status.response == 'ENABLE = 0':
        enable = bb_enable()

        if not enable.success:
            print(enable)
            return

    device_info = bb_get_device_info()
    print(device_info)

    reset = bb_reset()

    if not reset.success:
        print(reset)
        return

    #voltage = bb_measure_voltage()
    #print('voltage', voltage)

    #zauto = bb_measure_zauto()
    #print('zauto', zauto)

    #rcdi = bb_measure_rcdi('+')
    #print('rcdi', rcdi)

    #riso = bb_measure_riso('L1/PE')
    #print('riso', riso)

    #rlow = bb_measure_rlow()
    #print('rlow', rlow)

    bb_disable()


if __name__ == "__main__":
    main()
