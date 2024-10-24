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

from pymodbus.payload import BinaryPayloadDecoder
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
        if self.reg_type == 'coil':
            rr = client.read_coils(self.offset, self.count)
            coils = True
        elif self.reg_type == 'discrete':
            rr = client.read_discrete_inputs(self.offset, self.count)
            coils = True
        elif self.reg_type == 'holding':
            rr = client.read_holding_registers(self.offset, self.count)
        elif self.reg_type == 'input':
            rr = client.read_input_registers(self.offset, self.count)

        if rr.isError():
            print(f"Received Modbus library error({rr})")
            raise Exception(rr)
        if isinstance(rr, ExceptionResponse):
            print(f"Received Modbus library exception ({rr})")
            return []

        if coils:
            d = rr.bits
        else:
            d = BinaryPayloadDecoder.fromRegisters(rr.registers, byteorder=Endian.BIG)

        result = []
        off = self.offset
        for fn, name in self.payload_converter:
            #breakpoint()
            len_, val = fn(d)
            if name != "_":
                result.append((off, name, val))
            off += len_

        return result

def u16(d):
    return 1, d.decode_16bit_uint()

def u32(d):
    return 2, d.decode_32bit_uint()

def f32(d):
    return 2, d.decode_32bit_float()


def u16_block(len_, rjust=10):
    return lambda d: (len_ * 2, " ".join(f'{x}'.rjust(rjust) for x in [d.decode_16bit_uint() for i in range(len_)]))

def u32_block(len_, rjust=10):
    return lambda d: (len_ * 2, " ".join(f'{x}'.rjust(rjust) for x in [d.decode_32bit_uint() for i in range(len_)]))

def f32_block(len_, rjust=10):
    return lambda d: (len_ * 2, " ".join(f'{x:.3f}'.rjust(rjust) for x in [d.decode_32bit_float() for i in range(len_)]))


def bit(d):
    return 1, d.pop()

def str_(len_):
    def inner(d):
        b = d.decode_string(len_)
        return len_ // 2, f'{repr(b.strip(b'\x00').decode('ascii'))} (0x{b.hex(' ', 2).replace(' ', ' 0x')})'

    return inner

WARP_REGISTER_MAP = [
    RegisterBlock('input_regs', 'input', 0, 14, [
        (u32, "table_version"),
        (u32, "firmware_major"),
        (u32, "firmware_minor"),
        (u32, "firmware_patch"),
        (u32, "firmware_build_ts"),
        (u32, "box_id"),
        (u32, "uptime"),
    ]),

    RegisterBlock('evse_input_regs', 'input', 1000, 52, [
        (u32, "iec_state"),
        (u32, "charger_state"),
        (u32, "current_user"),
        (u32, "start_time_min"),
        (u32, "charging_time_sec"),
        (u32, "max_current"),
        (u32_block(10), 'slots_00'),
        (u32_block(10), 'slots_10'),
    ]),

    RegisterBlock('meter_input_regs', 'input', 2000, 10, [
        (u32, "meter_type"),
        (f32, "power"),
        (u32, "energy_absolute"),
        (u32, "energy_relative"),
        (u32, "energy_this_charge"),
    ]),

    RegisterBlock('meter_all_values_input_regs', 'input', 2100, 80, [
        (f32_block(10), 'meter_values_00'),
        (f32_block(10), 'meter_values_10'),
        (f32_block(10), 'meter_values_20'),
        (f32_block(10), 'meter_values_30'),
    ]),

    RegisterBlock('meter_all_values_input_regs_2', 'input', 2100 + 80, 90, [
        (f32_block(10), 'meter_values_40'),
        (f32_block(10), 'meter_values_50'),
        (f32_block(10), 'meter_values_60'),
        (f32_block(10), 'meter_values_70'),
        (f32_block(5), 'meter_values_80'),
    ]),

    RegisterBlock('nfc_input_regs', 'input', 4000, 12, [
        (str_(20), "tag_id"),
        (u32, "tag_id_age"),
    ]),

    RegisterBlock('holding_regs', 'holding', 0, 2, [
        (u32, "reboot"),
    ]),

    RegisterBlock('evse_holding_regs', 'holding', 1000, 8, [
        (u32, "enable_charging"),
        (u32, "allowed_current"),
        (u32, "led_blink_state"),
        (u32, "led_blink_duration"),
    ]),

    RegisterBlock('meter_holding_regs', 'holding', 2000, 2, [
        (u32, "trigger_reset")
    ]),

    RegisterBlock('discrete_inputs', 'discrete', 0, 6, [
        (bit, "evse"),
        (bit, "meter"),
        (bit, "meter_phases"),
        (bit, "meter_all_values"),
        (bit, "cp_disconnect"),
        (bit, "nfc"),
    ]),

    RegisterBlock('meter_discrete_inputs', 'discrete', 2100, 6, [
        (bit, "phase_one_connected"),
        (bit, "phase_two_connected"),
        (bit, "phase_three_connected"),
        (bit, "phase_one_active"),
        (bit, "phase_two_active"),
        (bit, "phase_three_active"),
    ]),

    RegisterBlock('evse_coils', 'coil', 1000, 2, [
        (bit, "enable_charging"),
        (bit, "autostart_button"),
    ]),
]

KEBA_REGISTER_MAP = [
    RegisterBlock('keba_read_general', 'holding', 1000, 48, [
        (u32, "charging_state"),
        (u32, "_"),
        (u32, "cable_state"),
        (u32, "error_code"),
 * 3 * [(u32, "currents")],
        (u32, "serial_number"),
        (u32, "features"),
        (u32, "firmware_version"),
        (u32, "power"),
 * 7 * [(u32, "_")],
        (u32, "total_energy"),
        (u32, "_"),
 * 3 * [(u32, "voltages")],
        (u32, "power_factor"),
    ]),

    RegisterBlock('keba_read_max', 'holding', 1100, 12, [
        (u32, "max_current"),
 * 4 * [(u32, "_")],
        (u32, "max_hardware_current")
    ]),

    RegisterBlock('keba_read_charge', 'holding', 1500, 4, [
        (u32, "rfid_tag"),
        (u32, "charged_energy")
    ]),

    RegisterBlock('keba_read_phase_switch', 'holding', 1550, 4, [
        (u32, "phase_switching_source"),
        (u32, "phase_switching_state")
    ]),

    RegisterBlock('keba_read_failsafe', 'holding', 1600, 4, [
        (u32, "failsafe_current"),
        (u32, "failsafe_timeout")
    ]),

    RegisterBlock('keba_write', 'holding', 5004, 17, [
        (u16, "set_charging_current"),
 * 5 * [(u16, "_")],
        (u16, "set_energy"),
        (u16, "_"),
        (u16, "unlock_plug"),
        (u16, "_"),
        (u16, "enable_station"),
        (u16, "_"),
        (u16, "failsafe_current"),
        (u16, "_"),
        (u16, "failsafe_timeout"),
        (u16, "_"),
        (u16, "failsafe_persist"),
    ]),

    RegisterBlock('keba_write_phase_switch', 'holding', 5050, 3, [
        (u16, "phase_switching_source"),
        (u16, "_"),
        (u16, "phase_switching_state"),
    ]),

]

BENDER_REGISTER_MAP = [
    RegisterBlock('bender_general', 'holding', 100, 53, [
        (str_(4), "firmware_version"),
        (u32, "_"),
        (u16, "ocpp_cp_state"),
 * 4 * [(u32, "errorcodes")],
 * 7 * [(u16, "_")],
        (str_(4), "protocol_version"),
        (u16, "vehicle_state"),
        (u16, "vehicle_state_hex"),
        (u16, "chargepoint_available"),
 * 6 * [(u16, "_")],
        (u16, "safe_current"),
        (u16, "comm_timeout"),
        (u16, "hardware_curr_limit"),
        (u16, "operator_curr_limit"),
        (u16, "rcmb_mode"),
        (u16, "rcmb_rms_int"),
        (u16, "rcmb_rms_frac"),
        (u16, "rcmb_dc_int"),
        (u16, "rcmb_dc_frac"),
        (u16, "relays_state"),
        (u16, "device_id"),
        (str_(20), "charger_model"),
        (u16, "pluglock_detected"),
    ]),

    RegisterBlock('bender_phases', 'holding', 200, 28, [
 * 3 * [(u32, "energy")],
 * 3 * [(u32, "power")],
 * 3 * [(u32, "current")],
        (u32, "total_energy"),
        (u32, "total_power"),
 * 3 * [(u32, "voltage")],
    ]),

    RegisterBlock('bender_dlm', 'holding', 600, 36, [
        (u16, "dlm_mode"),
 * 9 * [(u16, "_")],
 * 3 * [(u16, "evse_limit")],
 * 3 * [(u16, "operator_evse_limit")],
 * 4 * [(u16, "_")],
        (u16, "external_meter_support"),
        (u16, "number_of_slaves"),
 * 8 * [(u16, "_")],
 * 3 * [(u16, "overall_current_applied")],
 * 3 * [(u16, "overall_current_available")],
    ]),

    RegisterBlock('bender_charge', 'holding', 700, 46, [
 * 9 * [(u16, "_")],
        (u16, "wh_charged"),
        (u16, "current_signaled"),
        (u32, "start_time"),
        (u16, "charge_duration"),
        (u32, "end_time"),
        (u16, "minimum_current_limit"),
        (u32, "ev_required_energy"),
        (u16, "ev_max_curr"),
        (u32, "charged_energy"),
        (u32, "charge_duration_new"),
        (str_(20), "user_id"),
* 10 * [(u16, "_")],
        (str_(12), "evcc_id")
    ]),

    RegisterBlock('bender_hems', 'holding', 1000, 1, [
        (u16, "hems_limit"),
    ]),

    RegisterBlock('bender_write_uid', 'holding', 1110, 10, [
        (str_(20), "user_id"),
    ]),
]

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

        regs = {}
        last_regs = {}
        i = 0
        try:
            while True:
                result = table[i].read(client)
                regs[table[i].name] = result
                i += 1

                max_offset_len = len(str(max([offset for v in regs.values() for offset, name, value in v])))
                max_name_len = max([len(name) for v in regs.values() for offset, name, value in v])

                if i == len(table):
                    print("\033[H\033[2J", end="")
                    for k, v in regs.items():
                        print(f"=== {k} ===")
                        for i, tup in enumerate(v):
                            offset, name, value = tup
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
                            print(f"    {off_indent}{offset} {name}{name_indent} {value}")
                        print("")

                    i = 0
                    time.sleep(1)
                    last_regs = copy.deepcopy(regs)
                    regs = {}

        except Exception as e:
            print(e)
            client.close()
            time.sleep(1)

if __name__ == "__main__":
    main()
