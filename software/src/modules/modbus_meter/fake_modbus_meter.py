#!/usr/bin/env python
# -*- coding: utf-8 -*-

HOST = "localhost"
PORT = 4223

import time
import sys
from struct import pack, unpack

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_rs485 import BrickletRS485

# Callback function for Modbus master write single register response callback
def cb_modbus_slave_read_input_registers_request(rs485, uid, request_id, starting_address, count):
    regs = [0, 0]
    regs[1], regs[0] = unpack('<HH', pack('<f', 42.23))
    rs485.modbus_slave_answer_read_input_registers_request(request_id, regs)

# Print incoming enumeration
def cb_enumerate(ipcon, uid, connected_uid, position, hardware_version, firmware_version,
                 device_identifier, enumeration_type):
    if device_identifier != 277:
        return

    rs485 = BrickletRS485(uid, ipcon)

    print("Found RS485 bricklet {}".format(uid))

    rs485.set_mode(rs485.MODE_MODBUS_SLAVE_RTU)
    rs485.set_modbus_configuration(1, 1000)
    rs485.set_rs485_configuration(9600, 0, 1, 8, 0)
    rs485.register_callback(rs485.CALLBACK_MODBUS_SLAVE_READ_INPUT_REGISTERS_REQUEST,
                            lambda *args: cb_modbus_slave_read_input_registers_request(rs485, uid, *args))

if __name__ == "__main__":
    ipcon = IPConnection()
    ipcon.connect(HOST, PORT)
    # Register Enumerate Callback
    ipcon.register_callback(IPConnection.CALLBACK_ENUMERATE, lambda *args: cb_enumerate(ipcon, *args))

    # Trigger Enumerate
    ipcon.enumerate()
    print("!!!Reset both ESP Bricks!!!")
    while True:
       time.sleep(1)
    #input("Press key to exit\n") # Use raw_input() in Python 2
    #ipcon.disconnect()
