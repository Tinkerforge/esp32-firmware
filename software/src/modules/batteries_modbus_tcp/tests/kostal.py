#!/usr/bin/env -S uv run --script
#
# /// script
# dependencies = [
#     "pymodbus == 3.12.1",
# ]
# ///

import logging
import struct
import threading
import time
from argparse import ArgumentParser
from pymodbus import pymodbus_apply_logging_config
from pymodbus.server import StartTcpServer
from pymodbus.datastore import ModbusSparseDataBlock, ModbusDeviceContext, ModbusServerContext


class DataBlock(ModbusSparseDataBlock):
    def __init__(self):
        super().__init__()

        self.last_write_1034_1038_1040 = None
        self.last_write_1280_1282 = None

    def getValues(self, address, count=1):
        values = super().getValues(address, count)
        print('getValues', address - 1, count, '->', values)
        return values

    def getValues_plain(self, address, count=1):
        return super().getValues(address, count)

    def setValues(self, address, values):
        print('setValues', address - 1, values)
        super().setValues(address, values)

        if address - 1 in [1034, 1038, 1040]:
            self.last_write_1034_1038_1040 = time.monotonic()
        elif address - 1 in [1280, 1282]:
            self.last_write_1280_1282 = time.monotonic()

        setpoint = unpack_float32be(self.getValues_plain(1034 + 1, 2))
        print('setpoint@1034 =', setpoint)

        maximum_charge_power = unpack_float32be(self.getValues_plain(1038 + 1, 2))
        print('maximum_charge_power@1038 =', maximum_charge_power)

        maximum_discharge_power = unpack_float32be(self.getValues_plain(1040 + 1, 2))
        print('maximum_discharge_power@1040 =', maximum_discharge_power)

        if 1280 + 1 in self.values:
            max_battery_charge_power = unpack_float32be(self.getValues_plain(1280 + 1, 2))
            print('max_battery_charge_power@1280 =', max_battery_charge_power)

        if 1282 + 1 in self.values:
            max_battery_discharge_power = unpack_float32be(self.getValues_plain(1282 + 1, 2))
            print('max_battery_discharge_power@1282 =', max_battery_discharge_power)

    def setValues_plain(self, address, values):
        super().setValues(address, values)


def pack_float32be(value):
    return struct.unpack('>2H', struct.pack('>f', value))


def unpack_float32be(registers):
    return struct.unpack('>f', struct.pack('>2H', *registers))[0]


def thread_loop(data):
    timeout = 30

    while True:
        if data.last_write_1034_1038_1040 != None and data.last_write_1034_1038_1040 + timeout < time.monotonic():
            print('Resetting registers 1034, 1038 and 1040')
            data.setValues_plain(1034 + 1, [*pack_float32be(0)])
            data.setValues_plain(1038 + 1, [*pack_float32be(1000)])
            data.setValues_plain(1040 + 1, [*pack_float32be(1000)])
            data.last_write_1034_1038_1040 = None

        if data.last_write_1280_1282 != None and data.last_write_1280_1282 + timeout < time.monotonic():
            print('Resetting registers 1280 and 1282')
            data.setValues_plain(1280 + 1, [*pack_float32be(1000)])
            data.setValues_plain(1282 + 1, [*pack_float32be(1000)])
            data.last_write_1280_1282 = None

        time.sleep(0.25)


def main():
    parser = ArgumentParser()
    parser.add_argument('-d', '--debug', action='store_true')
    parser.add_argument('-p', '--port', type=int, default=5020)
    parser.add_argument('model', choices=['plenticore-plus-g2', 'plenticore-g3'])

    args = parser.parse_args()

    if args.debug:
        print('Enabling debug logging')
        pymodbus_apply_logging_config(logging.DEBUG)
        logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s %(message)s')

    data = DataBlock()

    if args.model == 'plenticore-plus-g2':
        data.setValues_plain(
            5 + 1,
            [
                1,  # Modbus Byte Order, big endian
            ],
        )

        data.setValues_plain(
            1034 + 1,
            [
                *pack_float32be(0),  # Battery charge power DC setpoint absolute [W]
            ],
        )

        data.setValues_plain(
            1038 + 1,
            [
                *pack_float32be(1000),  # Battery maximum charge power limit absolute [W]
            ],
        )

        data.setValues_plain(
            1040 + 1,
            [
                *pack_float32be(1000),  # Battery maximum discharge power limit absolute [W]
            ],
        )
    elif args.model == 'plenticore-g3':
        data.setValues_plain(
            5 + 1,
            [
                1,  # Modbus Byte Order, big endian
            ],
        )

        data.setValues_plain(
            1034 + 1,
            [
                *pack_float32be(0),  # Battery charge power DC setpoint absolute [W]
            ],
        )

        data.setValues_plain(
            1038 + 1,
            [
                *pack_float32be(1000),  # Battery maximum charge power limit absolute [W]
            ],
        )

        data.setValues_plain(
            1040 + 1,
            [
                *pack_float32be(1000),  # Battery maximum discharge power limit absolute [W]
            ],
        )

        data.setValues_plain(
            1280 + 1,
            [
                *pack_float32be(1000),  # Max battery charge power [W]
            ],
        )

        data.setValues_plain(
            1282 + 1,
            [
                *pack_float32be(1000),  # Max battery discharge power [W]
            ],
        )

    thread = threading.Thread(target=thread_loop, args=(data,), daemon=True)
    thread.start()

    device_ctx = ModbusDeviceContext(hr=data)
    server_ctx = ModbusServerContext(devices=device_ctx, single=True)

    print(f'Using model {args.model}')
    print(f'Listening on port {args.port}')
    StartTcpServer(server_ctx, address=('', args.port))


if __name__ == '__main__':
    main()
