#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from modbus_tcp_server.network import ModbusTCPServer
from modbus_tcp_server.data_source import BaseDataSource

import time

class DB(BaseDataSource):
    def get_holding_register(self, unit_id: int, address: int) -> int:
        print("get_holding_register", unit_id, address)
        return address

    def get_analog_input(self, unit_id: int, address: int) -> int:
        print("get_analog_input", unit_id, address)
        return address

    def get_discrete_input(self, unit_id: int, address: int) -> bool:
        print("get_discrete_input", unit_id, address)
        return True

    def get_coil(self, unit_id: int, address: int) -> bool:
        print("get_coil", unit_id, address)
        return True

    def set_holding_register(self, unit_id: int, address: int, value: int) -> None:
        print("set_holding_reegister", unit_id, address, value)

    def set_coil(self, unit_id: int, address: int, value: bool) -> None:
        print("set_coil", unit_id, address, value)

db = DB()
server = ModbusTCPServer('0.0.0.0', 502, db).start()

while True:
    time.sleep(1)
