import struct

from pymodbus.client import ModbusTcpClient
from pymodbus.exceptions import ModbusException


class RegisterBlock:
    def __init__(self, name, reg_type, reg_offset, reg_count, payload_converter):
        self.name = name
        self.reg_type = reg_type
        self.offset = reg_offset
        self.count = reg_count
        self.payload_converter = payload_converter

    def _read(self, client: ModbusTcpClient, offset, count):
        read_fn = {
            'coil': client.read_coils,
            'discrete': client.read_discrete_inputs,
            'holding': client.read_holding_registers,
            'input': client.read_input_registers,
        }[self.reg_type]
        read_bits = self.reg_type in ('coil', 'discrete')
        max_read = 2000 if read_bits else 124

        values = []
        while count > 0:
            to_read = min(max_read, count)
            response = read_fn(offset, count=to_read)
            if response.isError():
                raise ModbusException(f'{self.name}: {self.reg_type} read at {offset}, count {to_read}: {response}')

            data = response.bits if read_bits else response.registers
            if len(data) < to_read:
                raise ModbusException(f'{self.name}: expected {to_read} values at {offset}, got {len(data)}')
            values.extend(data[:to_read])
            offset += to_read
            count -= to_read

        return values

    def read(self, client: ModbusTcpClient):
        values = self._read(client, self.offset, self.count)
        result = []
        end = 0
        for (width, decode), name in self.payload_converter:
            start = end
            end = start + width
            if name == '_':
                continue
            value = decode(values[start:end])
            result.append((self.reg_type, self.offset + start, name, value))
        return result

    def read_single(self, field_name, client: ModbusTcpClient):
        offset = self.offset
        for (width, decode), name in self.payload_converter:
            if name == field_name:
                values = self._read(client, offset, width)
                return self.reg_type, offset, name, decode(values)
            offset += width
        raise KeyError(f'{self.name} has no field {field_name}')


def pack_registers(values):
    return struct.pack(f'>{len(values)}H', *values)


def unpack_registers(fmt, values):
    return struct.unpack(f'>{fmt}', pack_registers(values))


def u16():
    return 1, lambda values: values[0]


def u32():
    return 2, lambda values: unpack_registers('I', values)[0]


def f32():
    return 2, lambda values: unpack_registers('f', values)[0]


def u16_block(length):
    return length, list


def u32_block(length):
    return length * 2, lambda values: list(unpack_registers(f'{length}I', values))


def f32_block(length):
    return length * 2, lambda values: list(unpack_registers(f'{length}f', values))


def bit():
    return 1, lambda values: values[0]


def str_(length):
    return length // 2, pack_registers


WARP_REGISTER_MAP = [
    RegisterBlock('input_regs', 'input', 0, 16, [
        (u32(), "table_version"),
        (u32(), "firmware_major"),
        (u32(), "firmware_minor"),
        (u32(), "firmware_patch"),
        (u32(), "firmware_build_ts"),
        (u32(), "box_id"),
        (u32(), "uptime"),
        (u32(), "firmware_beta"),
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

    RegisterBlock('nfc_input_regs', 'input', 4000, 14, [
        (str_(20), "tag_id"),
        (u32(), "tag_id_age"),
        (str_(4), "tag_type"),
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

    RegisterBlock('discrete_inputs', 'discrete', 0, 9, [
        (bit(), "evse"),
        (bit(), "meter"),
        (bit(), "meter_phases"),
        (bit(), "meter_all_values"),
        (bit(), "phase_switch"),
        (bit(), "nfc"),
        (bit(), "gp_output_supported"),
        (bit(), "input_3_supported"),
        (bit(), "input_4_supported"),
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
        (u32(), "phase_switching_source_read"),
        (u32(), "phase_switching_state_read"),
    ]),

    RegisterBlock('keba_read_failsafe', 'holding', 1600, 4, [
        (u32(), "failsafe_current_read"),
        (u32(), "failsafe_timeout_read"),
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
        (u16(), "failsafe_current_write"),
        (u16(), "_"),
        (u16(), "failsafe_timeout_write"),
        (u16(), "_"),
        (u16(), "failsafe_persist"),
    ]),

    RegisterBlock('keba_write_phase_switch', 'holding', 5050, 3, [
        (u16(), "phase_switching_source_write"),
        (u16(), "_"),
        (u16(), "phase_switching_state_write"),
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
        (u16_block(5), "_"),
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
        (str_(20), "user_id_read"),
        (u16_block(10), "_"),
        (str_(12), "evcc_id"),
    ]),

    RegisterBlock('bender_hems', 'holding', 1000, 1, [
        (u16(), "hems_limit"),
    ]),

    RegisterBlock('bender_write_uid', 'holding', 1110, 10, [
        (str_(20), "user_id_write"),
    ]),
]


def read_all_regs(client, table):
    return {block.name: block.read(client) for block in table}
