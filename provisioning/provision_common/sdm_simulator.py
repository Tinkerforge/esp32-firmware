#!/usr/bin/env python3

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'provisioning')


HOST = "localhost"
PORT = 4223

from provisioning.tinkerforge.ip_connection import IPConnection
from provisioning.tinkerforge.bricklet_rs485 import BrickletRS485

import time
import sys
from struct import pack, unpack
import random

class SDMSimulator:
    def __init__(self, rs485_bricklet_uid):
        self.ipcon = IPConnection()
        self.ipcon.connect(HOST, PORT)

        self.rs485 = BrickletRS485(rs485_bricklet_uid, self.ipcon)
        self.rs485.set_mode(self.rs485.MODE_MODBUS_SLAVE_RTU)
        self.rs485.set_modbus_configuration(1, 10000)
        self.rs485.set_rs485_configuration(9600, 0, 1, 8, 0)
        self.rs485.register_callback(self.rs485.CALLBACK_MODBUS_SLAVE_READ_HOLDING_REGISTERS_REQUEST, self.cb_modbus_slave_read_holding_registers_request)
        self.rs485.register_callback(self.rs485.CALLBACK_MODBUS_SLAVE_READ_INPUT_REGISTERS_REQUEST, self.cb_modbus_slave_read_input_registers_request)
        self.rs485.register_callback(self.rs485.CALLBACK_MODBUS_SLAVE_WRITE_MULTIPLE_REGISTERS_REQUEST, self.cb_modbus_slave_write_multiple_registers_registers_request)

        # Internal
        self.system_type = 3.0    # can be overwritten by master
        self.password    = 1000.0 # set by master

        # Externaly changable
        self.regs = {
            1: 230.0,  # Voltage L1
            3: 230.0,  # Voltage L2
            5: 230.0,  # Voltage L3
            7: 29.5,   # Current L1
            9: 29.6,   # Current L2
            11: 29.5,  # Current L3
            13: 6751,  # Power L1
            15: 6841,  # Power L2
            17: 6840,  # Power L3
            19: 6770,  # Volt Amp L1
            21: 6861,  # Volt Amp L2
            23: 6860,  # Volt Amp L3
            25: 447,   # Reactive Power L1
            27: 386,   # Reactive Power L2
            29: 447,   # Reactive Power L3
            31: 1.0,   # Power Factor L1
            33: 1.0,   # Power Factor L2
            35: 1.0,   # Power Factor L3
            53: 53.0,  # Total system power
            73: 42.0,  # Import
            75: 42.0,  # Export
            343: 343.0 # Total kwh sum
        }
        self.default_value = 42.23    # default value for unset regs
        self.phases        = 3        # 3 or 1
        self.meter         = 'SDM630' # SDM630 or SDM72V2
        self.noise         = (0.99, 1.01) # set to (1.0, 1.0) for no noise

        self.log_write        = False
        self.log_read_holding = False
        self.log_read_input   = False

    def set_register(self, address, value):
        self.regs[address] = value

    def cb_modbus_slave_write_multiple_registers_registers_request(self, request_id, starting_address, regs):
        if self.log_write: print("modbus write in req id {0}, start {1}, regs {2}".format(request_id, starting_address, regs))
        if len(regs) == 2:
            value = unpack('!f', pack('!HH', *regs))[0]
            if starting_address in (15, 25): # 15 for SDM72V2, 25 for SDM630
                self.password = value
                if self.log_write: print(" ----> password '{0}' set".format(value))
            elif starting_address == 11:
                self.system_type = value
                if self.log_write: print(" ----> system type '{0}' set".format(value))
                
        self.rs485.modbus_slave_answer_write_multiple_registers_request(request_id)
        if self.log_write: print('')

    def cb_modbus_slave_read_holding_registers_request(self, request_id, starting_address, count):
        if self.log_read_holding: print("modbus holding in req id {0}, start {1}, count {2}".format(request_id, starting_address, count))

        if starting_address == 64515:
            if self.meter == 'SDM72V2':
                reg = [0x0089]
            else:
                reg = [0x0070]
        elif starting_address == 11: # system type
            reg = [0, 0]
            reg[1], reg[0] = unpack('<HH', pack('<f', self.system_type))
        else:
            reg = [0]*count

        self.rs485.modbus_slave_answer_read_holding_registers_request(request_id, reg)
        if self.log_read_holding: print("modbus holding out req id {0}, regs {1}".format(request_id, reg))
        if self.log_read_holding: print('')

    def cb_modbus_slave_read_input_registers_request(self, request_id, starting_address, count):
        if self.log_read_input: print("modbus input in req id {0}, start {1}, count {2}".format(request_id, starting_address, count))

        reg = [0, 0]
        value = self.regs.get(starting_address)
        if type(value) is not float:
            value = self.default_value

        reg[1], reg[0] = unpack('<HH', pack('<f', value*random.uniform(*self.noise)))

        self.rs485.modbus_slave_answer_read_input_registers_request(request_id, reg)
        if self.log_read_input: print("modbus input out req id {0}, regs {1}".format(request_id, reg))
        if self.log_read_input: print('')
 
if __name__ == "__main__":
    sdm_sim = SDMSimulator('W2J')
    input("Press key to exit\n")
