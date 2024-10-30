#!/usr/bin/env python3

import sys
import time
import binascii
import copy
import math

import pymodbus.client as ModbusClient
from pymodbus import (
    ExceptionResponse,
    FramerType,
    ModbusException,
    pymodbus_apply_logging_config,
)

from pymodbus.payload import BinaryPayloadDecoder, BinaryPayloadBuilder
from pymodbus.constants import Endian

class RegisterBlock:
    def __init__(self, name, reg_type, reg_offset, reg_count, payload_converter):
        self.name = name
        self.reg_type = reg_type
        self.offset = reg_offset
        self.count = reg_count
        self.payload_converter = payload_converter

    def read(self, client: ModbusClient.ModbusTcpClient):
        coils = False
        max_read = 0
        if self.reg_type == 'coil':
            read_fn = lambda offset, count: client.read_coils(offset, count)
            coils = True
            max_read = 2000
        elif self.reg_type == 'discrete':
            read_fn = lambda offset, count: client.read_discrete_inputs(offset, count)
            coils = True
            max_read = 2000
        elif self.reg_type == 'holding':
            read_fn = lambda offset, count: client.read_holding_registers(offset, count)
            max_read = 124
        elif self.reg_type == 'input':
            read_fn = lambda offset, count: client.read_input_registers(offset, count)
            max_read = 124

        read_regs = []

        count = self.count
        offset = self.offset
        while count > 0:
            to_read = min(max_read, count)

            rr = read_fn(offset, to_read)

            if rr.isError():
                print(f"Received Modbus library error({rr})")
                raise Exception(rr)
            if isinstance(rr, ExceptionResponse):
                print(f"Received Modbus library exception ({rr})")
                return []

            count -= to_read
            offset += to_read

            if coils:
                read_regs += rr.bits
            else:
                read_regs += rr.registers

        if not coils:
            read_regs = BinaryPayloadDecoder.fromRegisters(read_regs, byteorder=Endian.BIG)

        result = []
        off = self.offset
        for tup, name in self.payload_converter:
            len_, fn = tup
            val = fn(read_regs)
            if name != "_":
                result.append([self.reg_type, off, name, val])
            off += len_

        return result

    def read_single(self, block_idx, client: ModbusClient.ModbusTcpClient):
        coils = False
        max_read = 0
        if self.reg_type == 'coil':
            read_fn = lambda offset, count: client.read_coils(offset, count)
            coils = True
            max_read = 2000
        elif self.reg_type == 'discrete':
            read_fn = lambda offset, count: client.read_discrete_inputs(offset, count)
            coils = True
            max_read = 2000
        elif self.reg_type == 'holding':
            read_fn = lambda offset, count: client.read_holding_registers(offset, count)
            max_read = 124
        elif self.reg_type == 'input':
            read_fn = lambda offset, count: client.read_input_registers(offset, count)
            max_read = 124


        start_offset = self.offset + sum(tup[0] for tup, name in self.payload_converter[:block_idx])
        offset = start_offset
        count = self.payload_converter[block_idx][0][0]

        read_regs = []

        while count > 0:
            to_read = min(max_read, count)

            rr = read_fn(offset, to_read)

            if rr.isError():
                print(f"Received Modbus library error({rr})")
                raise Exception(rr)
            if isinstance(rr, ExceptionResponse):
                print(f"Received Modbus library exception ({rr})")
                return []

            count -= to_read
            offset += to_read

            if coils:
                read_regs += rr.bits
            else:
                read_regs += rr.registers

        if not coils:
            read_regs = BinaryPayloadDecoder.fromRegisters(read_regs, byteorder=Endian.BIG)

        tup, name = self.payload_converter[block_idx]
        len_, fn = tup

        return self.reg_type, start_offset, name, fn(read_regs)

def u16():
    return 1, lambda d: d.decode_16bit_uint()

def u32():
    return 2, lambda d: d.decode_32bit_uint()

def f32():
    return 2, lambda d: d.decode_32bit_float()


def u16_block(len_):
    return len_ * 2, lambda d: [d.decode_16bit_uint() for i in range(len_)]

def u32_block(len_):
    return len_ * 2, lambda d: [d.decode_32bit_uint() for i in range(len_)]

def f32_block(len_):
    return len_ * 2, lambda d: [d.decode_32bit_float() for i in range(len_)]


def bit():
    return 1, lambda d: d.pop(0)

def str_(len_):
    return len_ // 2, lambda d: d.decode_string(len_)

WARP_REGISTER_MAP = [
    RegisterBlock('input_regs', 'input', 0, 14, [
        (u32(), "table_version"),
        (u32(), "firmware_major"),
        (u32(), "firmware_minor"),
        (u32(), "firmware_patch"),
        (u32(), "firmware_build_ts"),
        (u32(), "box_id"),
        (u32(), "uptime"),
    ]),

    RegisterBlock('evse_input_regs', 'input', 1000, 52, [
        (u32(), "iec_state"),
        (u32(), "charger_state"),
        (u32(), "current_user"),
        (u32(), "start_time_min"),
        (u32(), "charging_time_sec"),
        (u32(), "max_current"),
        (u32_block(20), 'slots'),
    ]),

    RegisterBlock('meter_input_regs', 'input', 2000, 10, [
        (u32(), "meter_type"),
        (f32(), "power"),
        (f32(), "energy_absolute"),
        (f32(), "energy_relative"),
        (f32(), "energy_this_charge"),
    ]),

    RegisterBlock('meter_all_values_input_regs', 'input', 2100, 170, [
        (f32_block(85), 'meter_values'),
    ]),

    RegisterBlock('phase_switch_input_regs', 'input', 3100, 4, [
        (u32(), "phases_connected"),
        (u32(), "external_control_state"),
    ]),

    RegisterBlock('nfc_input_regs', 'input', 4000, 12, [
        (str_(20), "tag_id"),
        (u32(), "tag_id_age"),
    ]),

    RegisterBlock('holding_regs', 'holding', 0, 2, [
        (u32(), "reboot"),
    ]),

    RegisterBlock('evse_holding_regs', 'holding', 1000, 8, [
        (u32(), "enable_charging_deprecated"),
        (u32(), "allowed_current"),
        (u32(), "led_blink_state"),
        (u32(), "led_blink_duration"),
    ]),

    RegisterBlock('meter_holding_regs', 'holding', 2000, 2, [
        (u32(), "trigger_reset"),
    ]),

    RegisterBlock('phase_switch_holding_regs', 'holding', 3100, 2, [
        (u32(), "phases_wanted"),
    ]),


    RegisterBlock('discrete_inputs', 'discrete', 0, 6, [
        (bit(), "evse"),
        (bit(), "meter"),
        (bit(), "meter_phases"),
        (bit(), "meter_all_values"),
        (bit(), "phase_switch"),
        (bit(), "nfc"),
    ]),

    RegisterBlock('meter_discrete_inputs', 'discrete', 2100, 6, [
        (bit(), "phase_one_connected"),
        (bit(), "phase_two_connected"),
        (bit(), "phase_three_connected"),
        (bit(), "phase_one_active"),
        (bit(), "phase_two_active"),
        (bit(), "phase_three_active"),
    ]),

    RegisterBlock('evse_coils', 'coil', 1000, 2, [
        (bit(), "enable_charging"),
        (bit(), "autostart_button"),
    ]),
]

KEBA_REGISTER_MAP = [
    RegisterBlock('keba_read_general', 'holding', 1000, 48, [
        (u32(), "charging_state"),
        (u32(), "_"),
        (u32(), "cable_state"),
        (u32(), "error_code"),
        (u32_block(3), "currents"),
        (u32(), "serial_number"),
        (u32(), "features"),
        (u32(), "firmware_version"),
        (u32(), "power"),
        (u32_block(7), "_"),
        (u32(), "total_energy"),
        (u32(), "_"),
        (u32_block(3), "voltages"),
        (u32(), "power_factor"),
    ]),

    RegisterBlock('keba_read_max', 'holding', 1100, 12, [
        (u32(), "max_current"),
        (u32_block(4), "_"),
        (u32(), "max_hardware_current"),
    ]),

    RegisterBlock('keba_read_charge', 'holding', 1500, 4, [
        (u32(), "rfid_tag"),
        (u32(), "charged_energy"),
    ]),

    RegisterBlock('keba_read_phase_switch', 'holding', 1550, 4, [
        (u32(), "phase_switching_source"),
        (u32(), "phase_switching_state"),
    ]),

    RegisterBlock('keba_read_failsafe', 'holding', 1600, 4, [
        (u32(), "failsafe_current"),
        (u32(), "failsafe_timeout"),
    ]),

    RegisterBlock('keba_write', 'holding', 5004, 17, [
        (u16(), "set_charging_current"),
        (u16_block(5), "_"),
        (u16(), "set_energy"),
        (u16(), "_"),
        (u16(), "unlock_plug"),
        (u16(), "_"),
        (u16(), "enable_station"),
        (u16(), "_"),
        (u16(), "failsafe_current"),
        (u16(), "_"),
        (u16(), "failsafe_timeout"),
        (u16(), "_"),
        (u16(), "failsafe_persist"),
    ]),

    RegisterBlock('keba_write_phase_switch', 'holding', 5050, 3, [
        (u16(), "phase_switching_source"),
        (u16(), "_"),
        (u16(), "phase_switching_state"),
    ]),

]

BENDER_REGISTER_MAP = [
    RegisterBlock('bender_general', 'holding', 100, 53, [
        (str_(4), "firmware_version"),
        (u32(), "_"),
        (u16(), "ocpp_cp_state"),
        (u32_block(4), "errorcodes"),
        (u16_block(7), "_"),
        (str_(4), "protocol_version"),
        (u16(), "vehicle_state"),
        (u16(), "vehicle_state_hex"),
        (u16(), "chargepoint_available"),
        (u16_block(6), "_"),
        (u16(), "safe_current"),
        (u16(), "comm_timeout"),
        (u16(), "hardware_curr_limit"),
        (u16(), "operator_curr_limit"),
        (u16(), "rcmb_mode"),
        (u16(), "rcmb_rms_int"),
        (u16(), "rcmb_rms_frac"),
        (u16(), "rcmb_dc_int"),
        (u16(), "rcmb_dc_frac"),
        (u16(), "relays_state"),
        (u16(), "device_id"),
        (str_(20), "charger_model"),
        (u16(), "pluglock_detected"),
    ]),

    RegisterBlock('bender_phases', 'holding', 200, 28, [
        (u32_block(3), "energy"),
        (u32_block(3), "power"),
        (u32_block(3), "current"),
        (u32(), "total_energy"),
        (u32(), "total_power"),
        (u32_block(3), "voltage"),
    ]),

    RegisterBlock('bender_dlm', 'holding', 600, 36, [
        (u16(), "dlm_mode"),
        (u16_block(9), "_"),
        (u16_block(3), "evse_limit"),
        (u16_block(3), "operator_evse_limit"),
        (u16_block(4), "_"),
        (u16(), "external_meter_support"),
        (u16(), "number_of_slaves"),
        (u16_block(8), "_"),
        (u16_block(3), "overall_current_applied"),
        (u16_block(3), "overall_current_available"),
    ]),

    RegisterBlock('bender_charge', 'holding', 700, 46, [
        (u16_block(9), "_"),
        (u16(), "wh_charged"),
        (u16(), "current_signaled"),
        (u32(), "start_time"),
        (u16(), "charge_duration"),
        (u32(), "end_time"),
        (u16(), "minimum_current_limit"),
        (u32(), "ev_required_energy"),
        (u16(), "ev_max_curr"),
        (u32(), "charged_energy"),
        (u32(), "charge_duration_new"),
        (str_(20), "user_id"),
        (u16_block(10), "_"),
        (str_(12), "evcc_id"),
    ]),

    RegisterBlock('bender_hems', 'holding', 1000, 1, [
        (u16(), "hems_limit"),
    ]),

    RegisterBlock('bender_write_uid', 'holding', 1110, 10, [
        (str_(20), "user_id"),
    ]),
]

def read_all_regs(client, table):
    regs = {}
    for block in table:
        result = block.read(client)
        regs[block.name] = result

    return regs

class color:
    PURPLE = '\033[95m'
    CYAN = '\033[96m'
    DARKCYAN = '\033[36m'
    BLUE = '\033[94m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    INVERT = '\033[7m'
    END = '\033[0m'

def chunks(lst, n):
    """Yield successive n-sized chunks from lst."""
    for i in range(0, len(lst), n):
        yield lst[i:i + n]

def main():
    table = {
        'warp': WARP_REGISTER_MAP,
        'keba': KEBA_REGISTER_MAP,
        'bender': BENDER_REGISTER_MAP,
    }[sys.argv[2]]

    while True:
        client = ModbusClient.ModbusTcpClient(
            sys.argv[1],
            port=502,
            framer=FramerType.SOCKET)

        client.connect()

        last_regs = {}

        try:
            while True:
                regs = read_all_regs(client, table)

                max_offset_len = len(str(max([offset for v in regs.values() for type_, offset, name, value in v])))
                max_name_len = max([len(name) for v in regs.values() for type_, offset, name, value in v])

                print("\033[H\033[2J", end="")
                for k, v in regs.items():
                    print(f"=== {k} ===")
                    for i, tup in enumerate(v):
                        type_, offset, name, value = tup

                        if isinstance(value, bytes):
                            value = f'{repr(value.strip(b'\x00').decode('ascii'))} (0x{value.hex(' ', 2).replace(' ', ' 0x')})'
                            regs[k][i][2] = value
                        elif isinstance(value, list):
                            value = "\n".join(" ".join(f'{x:.3f}'.rjust(14) if isinstance(x, float) else f'{x:d}'.rjust(10) for x in y) for y in chunks(value, 10))
                            regs[k][i][2] = value
                        elif isinstance(value, float):
                            value = f'{value:.3f}'.rjust(14)
                            regs[k][i][2] = value
                        elif isinstance(value, int):
                            value = f'{value:d}'.rjust(10)
                            regs[k][i][2] = value

                        off_indent = " " * (max_offset_len - len(str(offset)))
                        name_indent = " " * (max_name_len - len(name))
                        if k in last_regs and (not isinstance(value, float) or not math.isnan(value)) and value != last_regs[k][i][2]:
                            if not isinstance(value, str):
                                value = color.INVERT + str(value) + color.END
                            else:
                                last_value = last_regs[k][i][2]
                                l = max(len(last_value), len(value))
                                last_value = last_value.ljust(l)
                                value = value.ljust(l)
                                result = ""
                                for old_c, new_c in zip(last_value, value):
                                    if old_c != new_c:
                                        result += color.INVERT + new_c + color.END
                                    else:
                                        result += new_c
                                value = result.replace(color.END + color.INVERT, "")
                        indent = len(f'    {off_indent}{offset} {name}{name_indent} ')
                        if isinstance(value, str):
                            value = value.replace("\n", "\n" + " " * indent)
                        print(f"    {off_indent}{offset} {name}{name_indent} {value}")
                    print("")

                time.sleep(1)
                last_regs = copy.deepcopy(regs)

        except Exception as e:
            print(e)
            client.close()
            time.sleep(1)

if __name__ == "__main__":
    main()
