#!/usr/bin/python3 -u

# https://github.com/sunspec/models/blob/master/json

import sys
import math
import argparse
import pymodbus

print('Using pymodbus version:', pymodbus.__version__)

if not pymodbus.__version__.startswith('3.'):
    print('Error: Available pymodbus version is too old, version 3.x.y required')
    sys.exit(1)

from pymodbus.client import ModbusTcpClient
from pymodbus.constants import Endian
from pymodbus.payload import BinaryPayloadDecoder
from pymodbus.pdu import ExceptionResponse
from pymodbus.exceptions import ModbusException

class ReaderError(Exception):
    pass

BASE_ADDRESSES = [40000, 50000, 0]
SUN_SPEC_ID = 0x53756e53
COMMON_MODEL_ID = 1

class Reader:
    def __init__(self, client, address, device_address):
        self.client = client
        self.address = address
        self.device_address = device_address

    def skip(self, length_registers):
        self.address += length_registers

    def read_int16(self):
        result = self.client.read_holding_registers(self.address, count=1, slave=self.device_address)

        if isinstance(result, (ExceptionResponse, ModbusException)):
            raise ReaderError(result)

        decoder = BinaryPayloadDecoder.fromRegisters(result.registers, Endian.BIG)
        result = decoder.decode_16bit_int()

        if result == -0x8000:
            result = None

        self.address += 1

        return result

    def read_uint16(self):
        result = self.client.read_holding_registers(self.address, count=1, slave=self.device_address)

        if isinstance(result, (ExceptionResponse, ModbusException)):
            raise ReaderError(result)

        decoder = BinaryPayloadDecoder.fromRegisters(result.registers, Endian.BIG)
        result = decoder.decode_16bit_uint()

        if result == 0xFFFF:
            result = None

        self.address += 1

        return result

    def read_int32(self):
        result = self.client.read_holding_registers(self.address, count=2, slave=self.device_address)

        if isinstance(result, (ExceptionResponse, ModbusException)):
            raise ReaderError(result)

        decoder = BinaryPayloadDecoder.fromRegisters(result.registers, Endian.BIG)
        result = decoder.decode_32bit_int()

        if result == -0x80000000:
            result = None

        self.address += 2

        return result

    def read_uint32(self):
        result = self.client.read_holding_registers(self.address, count=2, slave=self.device_address)

        if isinstance(result, (ExceptionResponse, ModbusException)):
            raise ReaderError(result)

        decoder = BinaryPayloadDecoder.fromRegisters(result.registers, Endian.BIG)
        result = decoder.decode_32bit_uint()

        if result == 0xFFFFFFFF:
            result = None

        self.address += 2

        return result

    def read_acc32(self):
        result = self.client.read_holding_registers(self.address, count=2, slave=self.device_address)

        if isinstance(result, (ExceptionResponse, ModbusException)):
            raise ReaderError(result)

        decoder = BinaryPayloadDecoder.fromRegisters(result.registers, Endian.BIG)
        result = decoder.decode_32bit_uint()

        if result == 0:
            result = None

        self.address += 2

        return result

    def read_float32(self):
        result = self.client.read_holding_registers(self.address, count=2, slave=self.device_address)

        if isinstance(result, (ExceptionResponse, ModbusException)):
            raise ReaderError(result)

        decoder = BinaryPayloadDecoder.fromRegisters(result.registers, Endian.BIG)
        result = decoder.decode_32bit_float()

        if math.isnan(result):
            result = None

        self.address += 2

        return result

    def read_string(self, length_bytes):
        length_registers = int(math.ceil(length_bytes / 2))
        result = self.client.read_holding_registers(self.address, count=length_registers, slave=self.device_address)

        if isinstance(result, (ExceptionResponse, ModbusException)):
            raise ReaderError(result)

        decoder = BinaryPayloadDecoder.fromRegisters(result.registers, Endian.BIG)
        self.address += length_registers

        return decoder.decode_string(length_bytes).rstrip(b'\x00').decode('utf-8')

def read_common_model(reader, device_address_ref):
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

        device_address_ref[0] = device_address

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

def read_inverter_model(reader):
    print('  Trying to read Inverter Model')

    try:
        ac_current = reader.read_float32()
        ac_current_a = reader.read_float32()
        ac_current_b = reader.read_float32()
        ac_current_c = reader.read_float32()

        print('    AC Current [A]:', ac_current)
        print('    AC Current A [A]:', ac_current_a)
        print('    AC Current B [A]:', ac_current_b)
        print('    AC Current C [A]:', ac_current_c)

        ac_voltage_a_b = reader.read_float32()
        ac_voltage_b_c = reader.read_float32()
        ac_voltage_c_a = reader.read_float32()
        ac_voltage_a_n = reader.read_float32()
        ac_voltage_b_n = reader.read_float32()
        ac_voltage_c_n = reader.read_float32()

        print('    AC Voltage A B [V]:', ac_voltage_a_b)
        print('    AC Voltage B C [V]:', ac_voltage_b_c)
        print('    AC Voltage C A [V]:', ac_voltage_c_a)
        print('    AC Voltage A N [V]:', ac_voltage_a_n)
        print('    AC Voltage B N [V]:', ac_voltage_b_n)
        print('    AC Voltage C N [V]:', ac_voltage_c_n)

        ac_real_power = reader.read_float32()

        print('    AC Real Power [W]:', ac_real_power)

        ac_frequency = reader.read_float32()

        print('    AC Frequency [Hz]:', ac_frequency)

        ac_apparent_power = reader.read_float32()

        print('    AC Apparent Power [VA]:', ac_apparent_power)

        ac_reactive_power = reader.read_float32()

        print('    AC Reactive Power [var]:', ac_reactive_power)

        ac_power_factor = reader.read_float32()

        print('    AC Power Factor [%]:', ac_power_factor)

        ac_real_energy = reader.read_float32()

        print('    AC Real Energy [Wh]:', ac_real_energy)

        dc_current = reader.read_float32()

        print('    DC Current [A]:', dc_current)

        dc_voltage = reader.read_float32()

        print('    DC Voltage [V]:', dc_voltage)

        dc_power = reader.read_float32()

        print('    DC Power [W]:', dc_power)

        reader.skip(22)

        print('    Skipping 22 registers')

        print('    Done')
    except Exception as e:
        print('    Error:', e)
        return False

    return True

def read_ac_meter_int_model(reader):
    print('  Trying to read AC Meter (integer) Model')

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

        ac_real_energy_exported = reader.read_acc32()
        ac_real_energy_exported_a = reader.read_acc32()
        ac_real_energy_exported_b = reader.read_acc32()
        ac_real_energy_exported_c = reader.read_acc32()
        ac_real_energy_imported = reader.read_acc32()
        ac_real_energy_imported_a = reader.read_acc32()
        ac_real_energy_imported_b = reader.read_acc32()
        ac_real_energy_imported_c = reader.read_acc32()
        ac_real_energy_sf = reader.read_int16()

        print('    AC Real Energy Exported [Wh]:', scale(ac_real_energy_exported, ac_real_energy_sf))
        print('    AC Real Energy Exported A [Wh]:', scale(ac_real_energy_exported_a, ac_real_energy_sf))
        print('    AC Real Energy Exported B [Wh]:', scale(ac_real_energy_exported_b, ac_real_energy_sf))
        print('    AC Real Energy Exported C [Wh]:', scale(ac_real_energy_exported_c, ac_real_energy_sf))
        print('    AC Real Energy Imported [Wh]:', scale(ac_real_energy_imported, ac_real_energy_sf))
        print('    AC Real Energy Imported A [Wh]:', scale(ac_real_energy_imported_a, ac_real_energy_sf))
        print('    AC Real Energy Imported B [Wh]:', scale(ac_real_energy_imported_b, ac_real_energy_sf))
        print('    AC Real Energy Imported C [Wh]:', scale(ac_real_energy_imported_c, ac_real_energy_sf))
        print('    AC Real Energy Scale Factor:', ac_real_energy_sf)

        ac_apparent_energy_exported = reader.read_acc32()
        ac_apparent_energy_exported_a = reader.read_acc32()
        ac_apparent_energy_exported_b = reader.read_acc32()
        ac_apparent_energy_exported_c = reader.read_acc32()
        ac_apparent_energy_imported = reader.read_acc32()
        ac_apparent_energy_imported_a = reader.read_acc32()
        ac_apparent_energy_imported_b = reader.read_acc32()
        ac_apparent_energy_imported_c = reader.read_acc32()
        ac_apparent_energy_sf = reader.read_int16()

        print('    AC Apparent Energy Exported [VAh]:', scale(ac_apparent_energy_exported, ac_apparent_energy_sf))
        print('    AC Apparent Energy Exported A [VAh]:', scale(ac_apparent_energy_exported_a, ac_apparent_energy_sf))
        print('    AC Apparent Energy Exported B [VAh]:', scale(ac_apparent_energy_exported_b, ac_apparent_energy_sf))
        print('    AC Apparent Energy Exported C [VAh]:', scale(ac_apparent_energy_exported_c, ac_apparent_energy_sf))
        print('    AC Apparent Energy Imported [VAh]:', scale(ac_apparent_energy_imported, ac_apparent_energy_sf))
        print('    AC Apparent Energy Imported A [VAh]:', scale(ac_apparent_energy_imported_a, ac_apparent_energy_sf))
        print('    AC Apparent Energy Imported B [VAh]:', scale(ac_apparent_energy_imported_b, ac_apparent_energy_sf))
        print('    AC Apparent Energy Imported C [VAh]:', scale(ac_apparent_energy_imported_c, ac_apparent_energy_sf))
        print('    AC Apparent Energy Scale Factor:', ac_apparent_energy_sf)

        ac_reactive_energy_imported_q1 = reader.read_acc32()
        ac_reactive_energy_imported_q1_a = reader.read_acc32()
        ac_reactive_energy_imported_q1_b = reader.read_acc32()
        ac_reactive_energy_imported_q1_c = reader.read_acc32()
        ac_reactive_energy_imported_q2 = reader.read_acc32()
        ac_reactive_energy_imported_q2_a = reader.read_acc32()
        ac_reactive_energy_imported_q2_b = reader.read_acc32()
        ac_reactive_energy_imported_q2_c = reader.read_acc32()
        ac_reactive_energy_exported_q3 = reader.read_acc32()
        ac_reactive_energy_exported_q3_a = reader.read_acc32()
        ac_reactive_energy_exported_q3_b = reader.read_acc32()
        ac_reactive_energy_exported_q3_c = reader.read_acc32()
        ac_reactive_energy_exported_q4 = reader.read_acc32()
        ac_reactive_energy_exported_q4_a = reader.read_acc32()
        ac_reactive_energy_exported_q4_b = reader.read_acc32()
        ac_reactive_energy_exported_q4_c = reader.read_acc32()
        ac_reactive_energy_sf = reader.read_int16()

        print('    AC Reactive Energy Imported Q1 [varh]:', scale(ac_reactive_energy_imported_q1, ac_reactive_energy_sf))
        print('    AC Reactive Energy Imported Q1 A [varh]:', scale(ac_reactive_energy_imported_q1_a, ac_reactive_energy_sf))
        print('    AC Reactive Energy Imported Q1 B [varh]:', scale(ac_reactive_energy_imported_q1_b, ac_reactive_energy_sf))
        print('    AC Reactive Energy Imported Q1 C [varh]:', scale(ac_reactive_energy_imported_q1_c, ac_reactive_energy_sf))
        print('    AC Reactive Energy Imported Q2 [varh]:', scale(ac_reactive_energy_imported_q2, ac_reactive_energy_sf))
        print('    AC Reactive Energy Imported Q2 A [varh]:', scale(ac_reactive_energy_imported_q2_a, ac_reactive_energy_sf))
        print('    AC Reactive Energy Imported Q2 B [varh]:', scale(ac_reactive_energy_imported_q2_b, ac_reactive_energy_sf))
        print('    AC Reactive Energy Imported Q2 C [varh]:', scale(ac_reactive_energy_imported_q2_c, ac_reactive_energy_sf))
        print('    AC Reactive Energy Exported Q3 [varh]:', scale(ac_reactive_energy_exported_q3, ac_reactive_energy_sf))
        print('    AC Reactive Energy Exported Q3 A [varh]:', scale(ac_reactive_energy_exported_q3_a, ac_reactive_energy_sf))
        print('    AC Reactive Energy Exported Q3 B [varh]:', scale(ac_reactive_energy_exported_q3_b, ac_reactive_energy_sf))
        print('    AC Reactive Energy Exported Q3 C [varh]:', scale(ac_reactive_energy_exported_q3_c, ac_reactive_energy_sf))
        print('    AC Reactive Energy Exported Q4 [varh]:', scale(ac_reactive_energy_exported_q4, ac_reactive_energy_sf))
        print('    AC Reactive Energy Exported Q4 A [varh]:', scale(ac_reactive_energy_exported_q4_a, ac_reactive_energy_sf))
        print('    AC Reactive Energy Exported Q4 B [varh]:', scale(ac_reactive_energy_exported_q4_b, ac_reactive_energy_sf))
        print('    AC Reactive Energy Exported Q4 C [varh]:', scale(ac_reactive_energy_exported_q4_c, ac_reactive_energy_sf))
        print('    AC Reactive Energy Scale Factor:', ac_reactive_energy_sf)

        events = reader.read_uint32()

        print('    Events:', events)

        print('    Done')
    except Exception as e:
        print('    Error:', e)
        return False

    return True

def read_ac_meter_float_model(reader):
    print('  Trying to read AC Meter (float) Model')

    try:
        ac_current = reader.read_float32()
        ac_current_a = reader.read_float32()
        ac_current_b = reader.read_float32()
        ac_current_c = reader.read_float32()

        print('    AC Current [A]:', ac_current)
        print('    AC Current A [A]:', ac_current_a)
        print('    AC Current B [A]:', ac_current_b)
        print('    AC Current C [A]:', ac_current_c)

        ac_voltage_l_n = reader.read_float32()
        ac_voltage_a_n = reader.read_float32()
        ac_voltage_b_n = reader.read_float32()
        ac_voltage_c_n = reader.read_float32()
        ac_voltage_l_l = reader.read_float32()
        ac_voltage_a_b = reader.read_float32()
        ac_voltage_b_c = reader.read_float32()
        ac_voltage_c_a = reader.read_float32()

        print('    AC Voltage L N [V]:', ac_voltage_l_n)
        print('    AC Voltage A N [V]:', ac_voltage_a_n)
        print('    AC Voltage B N [V]:', ac_voltage_b_n)
        print('    AC Voltage C N [V]:', ac_voltage_c_n)
        print('    AC Voltage L L [V]:', ac_voltage_l_l)
        print('    AC Voltage A B [V]:', ac_voltage_a_b)
        print('    AC Voltage B C [V]:', ac_voltage_b_c)
        print('    AC Voltage C A [V]:', ac_voltage_c_a)

        ac_frequency = reader.read_float32()

        print('    AC Frequency [Hz]:', ac_frequency)

        ac_real_power = reader.read_float32()
        ac_real_power_a = reader.read_float32()
        ac_real_power_b = reader.read_float32()
        ac_real_power_c = reader.read_float32()

        print('    AC Real Power [W]:', ac_real_power)
        print('    AC Real Power A [W]:', ac_real_power_a)
        print('    AC Real Power B [W]:', ac_real_power_b)
        print('    AC Real Power C [W]:', ac_real_power_c)

        ac_apparent_power = reader.read_float32()
        ac_apparent_power_a = reader.read_float32()
        ac_apparent_power_b = reader.read_float32()
        ac_apparent_power_c = reader.read_float32()

        print('    AC Apparent Power [VA]:', ac_apparent_power)
        print('    AC Apparent Power A [VA]:', ac_apparent_power_a)
        print('    AC Apparent Power B [VA]:', ac_apparent_power_b)
        print('    AC Apparent Power C [VA]:', ac_apparent_power_c)

        ac_reactive_power = reader.read_float32()
        ac_reactive_power_a = reader.read_float32()
        ac_reactive_power_b = reader.read_float32()
        ac_reactive_power_c = reader.read_float32()

        print('    AC Reactive Power [var]:', ac_reactive_power)
        print('    AC Reactive Power A [var]:', ac_reactive_power_a)
        print('    AC Reactive Power B [var]:', ac_reactive_power_b)
        print('    AC Reactive Power C [var]:', ac_reactive_power_c)

        ac_power_factor = reader.read_float32()
        ac_power_factor_a = reader.read_float32()
        ac_power_factor_b = reader.read_float32()
        ac_power_factor_c = reader.read_float32()

        print('    AC Power Factor [%]:', ac_power_factor)
        print('    AC Power Factor A [%]:', ac_power_factor_a)
        print('    AC Power Factor B [%]:', ac_power_factor_b)
        print('    AC Power Factor C [%]:', ac_power_factor_c)

        ac_real_energy_exported = reader.read_float32()
        ac_real_energy_exported_a = reader.read_float32()
        ac_real_energy_exported_b = reader.read_float32()
        ac_real_energy_exported_c = reader.read_float32()
        ac_real_energy_imported = reader.read_float32()
        ac_real_energy_imported_a = reader.read_float32()
        ac_real_energy_imported_b = reader.read_float32()
        ac_real_energy_imported_c = reader.read_float32()

        print('    AC Real Energy Exported [Wh]:', ac_real_energy_exported)
        print('    AC Real Energy Exported A [Wh]:', ac_real_energy_exported_a)
        print('    AC Real Energy Exported B [Wh]:', ac_real_energy_exported_b)
        print('    AC Real Energy Exported C [Wh]:', ac_real_energy_exported_c)
        print('    AC Real Energy Imported [Wh]:', ac_real_energy_imported)
        print('    AC Real Energy Imported A [Wh]:', ac_real_energy_imported_a)
        print('    AC Real Energy Imported B [Wh]:', ac_real_energy_imported_b)
        print('    AC Real Energy Imported C [Wh]:', ac_real_energy_imported_c)

        ac_apparent_energy_exported = reader.read_float32()
        ac_apparent_energy_exported_a = reader.read_float32()
        ac_apparent_energy_exported_b = reader.read_float32()
        ac_apparent_energy_exported_c = reader.read_float32()
        ac_apparent_energy_imported = reader.read_float32()
        ac_apparent_energy_imported_a = reader.read_float32()
        ac_apparent_energy_imported_b = reader.read_float32()
        ac_apparent_energy_imported_c = reader.read_float32()

        print('    AC Apparent Energy Exported [VAh]:', ac_apparent_energy_exported)
        print('    AC Apparent Energy Exported A [VAh]:', ac_apparent_energy_exported_a)
        print('    AC Apparent Energy Exported B [VAh]:', ac_apparent_energy_exported_b)
        print('    AC Apparent Energy Exported C [VAh]:', ac_apparent_energy_exported_c)
        print('    AC Apparent Energy Imported [VAh]:', ac_apparent_energy_imported)
        print('    AC Apparent Energy Imported A [VAh]:', ac_apparent_energy_imported_a)
        print('    AC Apparent Energy Imported B [VAh]:', ac_apparent_energy_imported_b)
        print('    AC Apparent Energy Imported C [VAh]:', ac_apparent_energy_imported_c)

        ac_reactive_energy_imported_q1 = reader.read_float32()
        ac_reactive_energy_imported_q1_a = reader.read_float32()
        ac_reactive_energy_imported_q1_b = reader.read_float32()
        ac_reactive_energy_imported_q1_c = reader.read_float32()
        ac_reactive_energy_imported_q2 = reader.read_float32()
        ac_reactive_energy_imported_q2_a = reader.read_float32()
        ac_reactive_energy_imported_q2_b = reader.read_float32()
        ac_reactive_energy_imported_q2_c = reader.read_float32()
        ac_reactive_energy_exported_q3 = reader.read_float32()
        ac_reactive_energy_exported_q3_a = reader.read_float32()
        ac_reactive_energy_exported_q3_b = reader.read_float32()
        ac_reactive_energy_exported_q3_c = reader.read_float32()
        ac_reactive_energy_exported_q4 = reader.read_float32()
        ac_reactive_energy_exported_q4_a = reader.read_float32()
        ac_reactive_energy_exported_q4_b = reader.read_float32()
        ac_reactive_energy_exported_q4_c = reader.read_float32()

        print('    AC Reactive Energy Imported Q1 [varh]:', ac_reactive_energy_imported_q1)
        print('    AC Reactive Energy Imported Q1 A [varh]:', ac_reactive_energy_imported_q1_a)
        print('    AC Reactive Energy Imported Q1 B [varh]:', ac_reactive_energy_imported_q1_b)
        print('    AC Reactive Energy Imported Q1 C [varh]:', ac_reactive_energy_imported_q1_c)
        print('    AC Reactive Energy Imported Q2 [varh]:', ac_reactive_energy_imported_q2)
        print('    AC Reactive Energy Imported Q2 A [varh]:', ac_reactive_energy_imported_q2_a)
        print('    AC Reactive Energy Imported Q2 B [varh]:', ac_reactive_energy_imported_q2_b)
        print('    AC Reactive Energy Imported Q2 C [varh]:', ac_reactive_energy_imported_q2_c)
        print('    AC Reactive Energy Exported Q3 [varh]:', ac_reactive_energy_exported_q3)
        print('    AC Reactive Energy Exported Q3 A [varh]:', ac_reactive_energy_exported_q3_a)
        print('    AC Reactive Energy Exported Q3 B [varh]:', ac_reactive_energy_exported_q3_b)
        print('    AC Reactive Energy Exported Q3 C [varh]:', ac_reactive_energy_exported_q3_c)
        print('    AC Reactive Energy Exported Q4 [varh]:', ac_reactive_energy_exported_q4)
        print('    AC Reactive Energy Exported Q4 A [varh]:', ac_reactive_energy_exported_q4_a)
        print('    AC Reactive Energy Exported Q4 B [varh]:', ac_reactive_energy_exported_q4_b)
        print('    AC Reactive Energy Exported Q4 C [varh]:', ac_reactive_energy_exported_q4_c)

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
        print('  Standard Model length:', length)

        if model_id == 113:
            if not read_inverter_model(reader):
                return False
        elif model_id in [201, 202, 203, 204]:
            if not read_ac_meter_int_model(reader):
                return False
        elif model_id in [211, 212, 213, 214]:
            if not read_ac_meter_float_model(reader):
                return False
        else:
            print('  Skipping Standard Model')
            reader.skip(length)

        print('  Done')
    except Exception as e:
        print('  Error:', e)
        return False

    return True

def discover(client, base_address, device_address, device_address_ref):
    reader = Reader(client, base_address, device_address)

    print('Using base address:', base_address)

    try:
        sun_spec_id = reader.read_uint32()

        if sun_spec_id != SUN_SPEC_ID:
            print('Sun Spec ID is wrong:', hex(sun_spec_id))
            return False

        print('Sun Spec ID found:', hex(sun_spec_id))

        if not read_common_model(reader, device_address_ref):
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
    parser.add_argument('-H', '--host', default='192.168.0.64')
    parser.add_argument('-p', '--port', type=int, default=502)
    parser.add_argument('-d', '--device-address', type=int)

    args = parser.parse_args()

    print('Using host:', args.host)
    print('Using port:', args.port)

    client = ModbusTcpClient(host=args.host, port=args.port)
    client.connect()

    valid_device_addresses = list(range(1, 248))

    for device_address in valid_device_addresses if args.device_address == None else [args.device_address]:
        print('============================================================')
        print('Using device address:', device_address)

        device_address_ref = [None]

        for base_address in BASE_ADDRESSES:
            print('------------------------------------------------------------')

            if discover(client, base_address, device_address, device_address_ref):
                break

        if device_address_ref[0] != None and device_address_ref[0] not in valid_device_addresses:
            print('Stopping discovery due to invalid device address')
            break

if __name__ == '__main__':
    main()
