#!/usr/bin/python3 -u

import time
import math
import argparse
from pymodbus.client.sync import ModbusTcpClient as ModbusClient
from pymodbus.constants import Endian
from pymodbus.payload import BinaryPayloadDecoder
from pymodbus.pdu import ExceptionResponse

class ReaderError(Exception):
    pass

BASE_ADDRESSES = [40000, 50000, 0]
SUN_SPEC_ID = 0x53756e53
COMMON_MODEL_ID = 1

class Reader:
    def __init__(self, client, address):
        self.client = client
        self.address = address

    def skip(self, length_registers):
        self.address += length_registers

    def read_int16(self):
        result = self.client.read_holding_registers(self.address, 1)

        if isinstance(result, ExceptionResponse):
            raise ReaderError(result)

        decoder = BinaryPayloadDecoder.fromRegisters(result.registers, Endian.Big)
        result = decoder.decode_16bit_int()

        if result == -0x8000:
            result = None

        self.address += 1

        return result

    def read_uint16(self):
        result = self.client.read_holding_registers(self.address, 1)

        if isinstance(result, ExceptionResponse):
            raise ReaderError(result)

        decoder = BinaryPayloadDecoder.fromRegisters(result.registers, Endian.Big)
        result = decoder.decode_16bit_uint()

        if result == 0xFFFF:
            result = None

        self.address += 1

        return result

    def read_int32(self):
        result = self.client.read_holding_registers(self.address, 2)

        if isinstance(result, ExceptionResponse):
            raise ReaderError(result)

        decoder = BinaryPayloadDecoder.fromRegisters(result.registers, Endian.Big)
        result = decoder.decode_32bit_int()

        if result == -0x80000000:
            result = None

        self.address += 2

        return result

    def read_uint32(self):
        result = self.client.read_holding_registers(self.address, 2)

        if isinstance(result, ExceptionResponse):
            raise ReaderError(result)

        decoder = BinaryPayloadDecoder.fromRegisters(result.registers, Endian.Big)
        result = decoder.decode_32bit_uint()

        if result == 0xFFFFFFFF:
            result = None

        self.address += 2

        return result

    def read_string(self, length_bytes):
        length_registers = int(math.ceil(length_bytes / 2))
        result = self.client.read_holding_registers(self.address, length_registers)

        if isinstance(result, ExceptionResponse):
            raise ReaderError(result)

        decoder = BinaryPayloadDecoder.fromRegisters(result.registers, Endian.Big)
        self.address += length_registers

        return decoder.decode_string(length_bytes).rstrip(b'\x00').decode('utf-8')

def read_common_model(reader):
    print('Trying to read Common Model')

    try:
        model_id = reader.read_uint16()

        if model_id != COMMON_MODEL_ID:
            print('  Common Model ID is wrong:', model_id)
            return False

        print('  Common Model ID found:', model_id)

        length = reader.read_uint16()

        if length not in [65, 66]:
            print('  Common Model has wrong length:', length)
            return False

        print('  Common Model length:', length)

        manufacturer_name = reader.read_string(32)
        print('  Manufacturer Name:', manufacturer_name)

        model_name = reader.read_string(32)
        print('  Model Name:', model_name)

        options = reader.read_string(16)
        print('  Options:', options)

        version = reader.read_string(16)
        print('  Version:', version)

        serial_number = reader.read_string(32)
        print('  Serial Number:', serial_number)

        device_address = reader.read_uint16()
        print('  Device Address:', device_address)

        if length == 66:
            reader.skip(1)

        print('  Done')
    except Exception as e:
        print('  Error:', e)
        return False

    return True

def scale(value, factor):
    if value == None:
        return None

    result = value * 10 ** factor

    if factor < 0:
        result = round(result, abs(factor))

    return result

def read_meter_model(reader):
    print('  Trying to read Meter Model')

    try:
        ac_current = reader.read_int16()
        ac_current_a = reader.read_int16()
        ac_current_b = reader.read_int16()
        ac_current_c = reader.read_int16()
        ac_current_sf = reader.read_int16()

        print('    AC Current [A]:', scale(ac_current, ac_current_sf))
        print('    AC Current A [A]:', scale(ac_current_a, ac_current_sf))
        print('    AC Current B [A]:', scale(ac_current_b, ac_current_sf))
        print('    AC Current C [A]:', scale(ac_current_c, ac_current_sf))
        print('    AC Current Scale Factor:', ac_current_sf)

        ac_voltage_l_n = reader.read_int16()
        ac_voltage_a_n = reader.read_int16()
        ac_voltage_b_n = reader.read_int16()
        ac_voltage_c_n = reader.read_int16()
        ac_voltage_l_l = reader.read_int16()
        ac_voltage_a_b = reader.read_int16()
        ac_voltage_b_c = reader.read_int16()
        ac_voltage_c_a = reader.read_int16()
        ac_voltage_sf = reader.read_int16()

        print('    AC Voltage L N [V]:', scale(ac_voltage_l_n, ac_voltage_sf))
        print('    AC Voltage A N [V]:', scale(ac_voltage_a_n, ac_voltage_sf))
        print('    AC Voltage B N [V]:', scale(ac_voltage_b_n, ac_voltage_sf))
        print('    AC Voltage C N [V]:', scale(ac_voltage_c_n, ac_voltage_sf))
        print('    AC Voltage L L [V]:', scale(ac_voltage_l_l, ac_voltage_sf))
        print('    AC Voltage A B [V]:', scale(ac_voltage_a_b, ac_voltage_sf))
        print('    AC Voltage B C [V]:', scale(ac_voltage_b_c, ac_voltage_sf))
        print('    AC Voltage C A [V]:', scale(ac_voltage_c_a, ac_voltage_sf))
        print('    AC Voltage Scale Factor:', ac_voltage_sf)

        ac_frequency = reader.read_int16()
        ac_frequency_sf = reader.read_int16()

        print('    AC Frequency [Hz]:', scale(ac_frequency, ac_frequency_sf))
        print('    AC Frequency Scale Factor:', ac_frequency_sf)

        ac_real_power = reader.read_int16()
        ac_real_power_a = reader.read_int16()
        ac_real_power_b = reader.read_int16()
        ac_real_power_c = reader.read_int16()
        ac_real_power_sf = reader.read_int16()

        print('    AC Real Power [W]:', scale(ac_real_power, ac_real_power_sf))
        print('    AC Real Power A [W]:', scale(ac_real_power_a, ac_real_power_sf))
        print('    AC Real Power B [W]:', scale(ac_real_power_b, ac_real_power_sf))
        print('    AC Real Power C [W]:', scale(ac_real_power_c, ac_real_power_sf))
        print('    AC Real Power Scale Factor:', ac_real_power_sf)

        ac_apparent_power = reader.read_int16()
        ac_apparent_power_a = reader.read_int16()
        ac_apparent_power_b = reader.read_int16()
        ac_apparent_power_c = reader.read_int16()
        ac_apparent_power_sf = reader.read_int16()

        print('    AC Apparent Power [VA]:', scale(ac_apparent_power, ac_apparent_power_sf))
        print('    AC Apparent Power A [VA]:', scale(ac_apparent_power_a, ac_apparent_power_sf))
        print('    AC Apparent Power B [VA]:', scale(ac_apparent_power_b, ac_apparent_power_sf))
        print('    AC Apparent Power C [VA]:', scale(ac_apparent_power_c, ac_apparent_power_sf))
        print('    AC Apparent Power Scale Factor:', ac_apparent_power_sf)

        ac_reactive_power = reader.read_int16()
        ac_reactive_power_a = reader.read_int16()
        ac_reactive_power_b = reader.read_int16()
        ac_reactive_power_c = reader.read_int16()
        ac_reactive_power_sf = reader.read_int16()

        print('    AC Reactive Power [var]:', scale(ac_reactive_power, ac_reactive_power_sf))
        print('    AC Reactive Power A [var]:', scale(ac_reactive_power_a, ac_reactive_power_sf))
        print('    AC Reactive Power B [var]:', scale(ac_reactive_power_b, ac_reactive_power_sf))
        print('    AC Reactive Power C [var]:', scale(ac_reactive_power_c, ac_reactive_power_sf))
        print('    AC Reactive Power Scale Factor:', ac_reactive_power_sf)

        ac_power_factor = reader.read_int16()
        ac_power_factor_a = reader.read_int16()
        ac_power_factor_b = reader.read_int16()
        ac_power_factor_c = reader.read_int16()
        ac_power_factor_sf = reader.read_int16()

        print('    AC Power Factor [%]:', scale(ac_power_factor, ac_power_factor_sf))
        print('    AC Power Factor A [%]:', scale(ac_power_factor_a, ac_power_factor_sf))
        print('    AC Power Factor B [%]:', scale(ac_power_factor_b, ac_power_factor_sf))
        print('    AC Power Factor C [%]:', scale(ac_power_factor_c, ac_power_factor_sf))
        print('    AC Power Factor Scale Factor:', ac_power_factor_sf)

        ac_real_enegry_exported = reader.read_uint32()
        ac_real_enegry_exported_a = reader.read_uint32()
        ac_real_enegry_exported_b = reader.read_uint32()
        ac_real_enegry_exported_c = reader.read_uint32()
        ac_real_enegry_imported = reader.read_uint32()
        ac_real_enegry_imported_a = reader.read_uint32()
        ac_real_enegry_imported_b = reader.read_uint32()
        ac_real_enegry_imported_c = reader.read_uint32()
        ac_real_enegry_sf = reader.read_int16()

        print('    AC Real Enegry Exported [Wh]:', scale(ac_real_enegry_exported, ac_real_enegry_sf))
        print('    AC Real Enegry Exported A [Wh]:', scale(ac_real_enegry_exported_a, ac_real_enegry_sf))
        print('    AC Real Enegry Exported B [Wh]:', scale(ac_real_enegry_exported_b, ac_real_enegry_sf))
        print('    AC Real Enegry Exported C [Wh]:', scale(ac_real_enegry_exported_c, ac_real_enegry_sf))
        print('    AC Real Enegry Imported [Wh]:', scale(ac_real_enegry_imported, ac_real_enegry_sf))
        print('    AC Real Enegry Imported A [Wh]:', scale(ac_real_enegry_imported_a, ac_real_enegry_sf))
        print('    AC Real Enegry Imported B [Wh]:', scale(ac_real_enegry_imported_b, ac_real_enegry_sf))
        print('    AC Real Enegry Imported C [Wh]:', scale(ac_real_enegry_imported_c, ac_real_enegry_sf))
        print('    AC Real Enegry Scale Factor:', ac_real_enegry_sf)

        ac_apparent_enegry_exported = reader.read_uint32()
        ac_apparent_enegry_exported_a = reader.read_uint32()
        ac_apparent_enegry_exported_b = reader.read_uint32()
        ac_apparent_enegry_exported_c = reader.read_uint32()
        ac_apparent_enegry_imported = reader.read_uint32()
        ac_apparent_enegry_imported_a = reader.read_uint32()
        ac_apparent_enegry_imported_b = reader.read_uint32()
        ac_apparent_enegry_imported_c = reader.read_uint32()
        ac_apparent_enegry_sf = reader.read_int16()

        print('    AC Apparent Enegry Exported [VAh]:', scale(ac_apparent_enegry_exported, ac_apparent_enegry_sf))
        print('    AC Apparent Enegry Exported A [VAh]:', scale(ac_apparent_enegry_exported_a, ac_apparent_enegry_sf))
        print('    AC Apparent Enegry Exported B [VAh]:', scale(ac_apparent_enegry_exported_b, ac_apparent_enegry_sf))
        print('    AC Apparent Enegry Exported C [VAh]:', scale(ac_apparent_enegry_exported_c, ac_apparent_enegry_sf))
        print('    AC Apparent Enegry Imported [VAh]:', scale(ac_apparent_enegry_imported, ac_apparent_enegry_sf))
        print('    AC Apparent Enegry Imported A [VAh]:', scale(ac_apparent_enegry_imported_a, ac_apparent_enegry_sf))
        print('    AC Apparent Enegry Imported B [VAh]:', scale(ac_apparent_enegry_imported_b, ac_apparent_enegry_sf))
        print('    AC Apparent Enegry Imported C [VAh]:', scale(ac_apparent_enegry_imported_c, ac_apparent_enegry_sf))
        print('    AC Apparent Enegry Scale Factor:', ac_apparent_enegry_sf)

        # FIXME: spec says uint32, but 2147483648 is returned which is "not implemented" for int32
        ac_reactive_enegry_imported_q1 = reader.read_int32()
        ac_reactive_enegry_imported_q1_a = reader.read_int32()
        ac_reactive_enegry_imported_q1_b = reader.read_int32()
        ac_reactive_enegry_imported_q1_c = reader.read_int32()
        ac_reactive_enegry_imported_q2 = reader.read_int32()
        ac_reactive_enegry_imported_q2_a = reader.read_int32()
        ac_reactive_enegry_imported_q2_b = reader.read_int32()
        ac_reactive_enegry_imported_q2_c = reader.read_int32()
        ac_reactive_enegry_exported_q3 = reader.read_int32()
        ac_reactive_enegry_exported_q3_a = reader.read_int32()
        ac_reactive_enegry_exported_q3_b = reader.read_int32()
        ac_reactive_enegry_exported_q3_c = reader.read_int32()
        ac_reactive_enegry_exported_q4 = reader.read_int32()
        ac_reactive_enegry_exported_q4_a = reader.read_int32()
        ac_reactive_enegry_exported_q4_b = reader.read_int32()
        ac_reactive_enegry_exported_q4_c = reader.read_int32()
        ac_reactive_enegry_sf = reader.read_int16()

        print('    AC Reactive Enegry Imported Q1 [varh]:', scale(ac_reactive_enegry_imported_q1, ac_reactive_enegry_sf))
        print('    AC Reactive Enegry Imported Q1 A [varh]:', scale(ac_reactive_enegry_imported_q1_a, ac_reactive_enegry_sf))
        print('    AC Reactive Enegry Imported Q1 B [varh]:', scale(ac_reactive_enegry_imported_q1_b, ac_reactive_enegry_sf))
        print('    AC Reactive Enegry Imported Q1 C [varh]:', scale(ac_reactive_enegry_imported_q1_c, ac_reactive_enegry_sf))
        print('    AC Reactive Enegry Imported Q2 [varh]:', scale(ac_reactive_enegry_imported_q2, ac_reactive_enegry_sf))
        print('    AC Reactive Enegry Imported Q2 A [varh]:', scale(ac_reactive_enegry_imported_q2_a, ac_reactive_enegry_sf))
        print('    AC Reactive Enegry Imported Q2 B [varh]:', scale(ac_reactive_enegry_imported_q2_b, ac_reactive_enegry_sf))
        print('    AC Reactive Enegry Imported Q2 C [varh]:', scale(ac_reactive_enegry_imported_q2_c, ac_reactive_enegry_sf))
        print('    AC Reactive Enegry Exported Q3 [varh]:', scale(ac_reactive_enegry_exported_q3, ac_reactive_enegry_sf))
        print('    AC Reactive Enegry Exported Q3 A [varh]:', scale(ac_reactive_enegry_exported_q3_a, ac_reactive_enegry_sf))
        print('    AC Reactive Enegry Exported Q3 B [varh]:', scale(ac_reactive_enegry_exported_q3_b, ac_reactive_enegry_sf))
        print('    AC Reactive Enegry Exported Q3 C [varh]:', scale(ac_reactive_enegry_exported_q3_c, ac_reactive_enegry_sf))
        print('    AC Reactive Enegry Exported Q4 [varh]:', scale(ac_reactive_enegry_exported_q4, ac_reactive_enegry_sf))
        print('    AC Reactive Enegry Exported Q4 A [varh]:', scale(ac_reactive_enegry_exported_q4_a, ac_reactive_enegry_sf))
        print('    AC Reactive Enegry Exported Q4 B [varh]:', scale(ac_reactive_enegry_exported_q4_b, ac_reactive_enegry_sf))
        print('    AC Reactive Enegry Exported Q4 C [varh]:', scale(ac_reactive_enegry_exported_q4_c, ac_reactive_enegry_sf))
        print('    AC Reactive Enegry Scale Factor:', ac_reactive_enegry_sf)

        events = reader.read_uint32()

        print('    Events:', events)

        print('    Done')
    except Exception as e:
        print('    Error:', e)
        return False

    return True

def read_standard_model(reader):
    print('Trying to read Standard Model')

    try:
        model_id = reader.read_uint16()

        if model_id == None:
            print('  End')
            return None

        print('  Standard Model ID found:', model_id)

        length = reader.read_uint16()
        print('  Standard Model Model length:', length)

        if model_id in [201, 202, 203, 204]:
            if not read_meter_model(reader):
                return False
        else:
            print('  Skipping')
            reader.skip(length)

        print('  Done')
    except Exception as e:
        print('  Error:', e)
        return False

    return True

def discover(client, base_address):
    reader = Reader(client, base_address)

    print('Using base address:', base_address)

    try:
        sun_spec_id = reader.read_uint32()

        if sun_spec_id != SUN_SPEC_ID:
            print('Sun Spec ID is wrong:', hex(sun_spec_id))
            return False

        print('Sun Spec ID found:', hex(sun_spec_id))

        if not read_common_model(reader):
            return False

        result = read_standard_model(reader)

        while result != None:
            if not result:
                return False

            result = read_standard_model(reader)

        print('Done')
    except Exception as e:
        print('Error:', e)
        return False

    return True

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('host', nargs='?', default='192.168.0.64')

    args = parser.parse_args()

    print('Using host:', args.host)

    client = ModbusClient(host=args.host, port=502)

    for base_address in BASE_ADDRESSES:
        if discover(client, base_address):
            break

if __name__ == '__main__':
    main()
