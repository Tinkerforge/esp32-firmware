#!/usr/bin/env python3

import base64
import string
import json
import argparse
import os, sys
import subprocess
import tempfile
from shutil import which

import parttool

def find_gdb():
    path = which("xtensa-esp32-elf-gdb")
    if path is not None:
        return path

    path = which("pio")
    if path is not None:
        return path + " pkg exec xtensa-esp32-elf-gdb --"

    return None

PREFIX = b"___tf_coredump_info_start___"
SUFFIX = b"___tf_coredump_info_end___"
EXTRA_INFO_HEADER = b'\xA5\x02\x00\x00EXTRA_INFO'

def extra_info_reg_name(reg):
    return {
        232: 'EXCCAUSE',
        238: 'EXCVADDR',
        177: 'EPC1',
        178: 'EPC2',
        179: 'EPC3',
        180: 'EPC4',
        181: 'EPC5',
        182: 'EPC6',
        183: 'EPC7',
        194: 'EPS2',
        195: 'EPS3',
        196: 'EPS4',
        197: 'EPS5',
        198: 'EPS6',
        199: 'EPS7'
    }.get(reg, 'unknown register')

# https://github.com/espressif/esp-coredump/blob/08f92cf36e7a649650a8b722cc96217026fd0174/esp_coredump/corefile/xtensa.py#L13-L72
INVALID_CAUSE_VALUE = 0xFFFF
XCHAL_EXCCAUSE_NUM = 64

# Exception cause dictionary to get translation of exccause register
# From 4.4.1.5 table 4-64 Exception Causes of Xtensa
# Instruction Set Architecture (ISA) Reference Manual

XTENSA_EXCEPTION_CAUSE_DICT = {
    0: ('IllegalInstructionCause', 'Illegal instruction'),
    1: ('SyscallCause', 'SYSCALL instruction'),
    2: ('InstructionFetchErrorCause',
        'Processor internal physical address or data error during instruction fetch. (See EXCVADDR for more information)'),
    3: ('LoadStoreErrorCause',
        'Processor internal physical address or data error during load or store. (See EXCVADDR for more information)'),
    4: ('Level1InterruptCause', 'Level-1 interrupt as indicated by set level-1 bits in the INTERRUPT register'),
    5: ('AllocaCause', 'MOVSP instruction, if caller`s registers are not in the register file'),
    6: ('IntegerDivideByZeroCause', 'QUOS: QUOU, REMS: or REMU divisor operand is zero'),
    8: ('PrivilegedCause', 'Attempt to execute a privileged operation when CRING ? 0'),
    9: ('LoadStoreAlignmentCause', 'Load or store to an unaligned address. (See EXCVADDR for more information)'),
    12: ('InstrPIFDataErrorCause', 'PIF data error during instruction fetch. (See EXCVADDR for more information)'),
    13: ('LoadStorePIFDataErrorCause',
         'Synchronous PIF data error during LoadStore access. (See EXCVADDR for more information)'),
    14: ('InstrPIFAddrErrorCause', 'PIF address error during instruction fetch. (See EXCVADDR for more information)'),
    15: ('LoadStorePIFAddrErrorCause',
         'Synchronous PIF address error during LoadStore access. (See EXCVADDR for more information)'),
    16: ('InstTLBMissCause', 'Error during Instruction TLB refill. (See EXCVADDR for more information)'),
    17: ('InstTLBMultiHitCause', 'Multiple instruction TLB entries matched. (See EXCVADDR for more information)'),
    18: ('InstFetchPrivilegeCause',
         'An instruction fetch referenced a virtual address at a ring level less than CRING. (See EXCVADDR for more information)'),
    20: ('InstFetchProhibitedCause',
         'An instruction fetch referenced a page mapped with an attribute that does not permit instruction fetch (EXCVADDR).'),
    24: ('LoadStoreTLBMissCause', 'Error during TLB refill for a load or store. (See EXCVADDR for more information)'),
    25: ('LoadStoreTLBMultiHitCause',
         'Multiple TLB entries matched for a load or store. (See EXCVADDR for more information)'),
    26: ('LoadStorePrivilegeCause',
         'A load or store referenced a virtual address at a ring level less than CRING. (See EXCVADDR for more information)'),
    28: ('LoadProhibitedCause',
         'A load referenced a page mapped with an attribute that does not permit loads. (See EXCVADDR for more information)'),
    29: ('StoreProhibitedCause',
         'A store referenced a page mapped with an attribute that does not permit stores [Region Protection Option or MMU Option].'),
    32: ('Coprocessor0Disabled', 'Coprocessor 0 instruction when cp0 disabled'),
    33: ('Coprocessor1Disabled', 'Coprocessor 1 instruction when cp1 disabled'),
    34: ('Coprocessor2Disabled', 'Coprocessor 2 instruction when cp2 disabled'),
    35: ('Coprocessor3Disabled', 'Coprocessor 3 instruction when cp3 disabled'),
    36: ('Coprocessor4Disabled', 'Coprocessor 4 instruction when cp4 disabled'),
    37: ('Coprocessor5Disabled', 'Coprocessor 5 instruction when cp5 disabled'),
    38: ('Coprocessor6Disabled', 'Coprocessor 6 instruction when cp6 disabled'),
    39: ('Coprocessor7Disabled', 'Coprocessor 7 instruction when cp7 disabled'),
    INVALID_CAUSE_VALUE: (
        'InvalidCauseRegister', 'Invalid EXCCAUSE register value or current task is broken and was skipped'),
    # ESP panic pseudo reasons
    XCHAL_EXCCAUSE_NUM + 0: ('UnknownException', 'Unknown exception'),
    XCHAL_EXCCAUSE_NUM + 1: ('DebugException', 'Unhandled debug exception'),
    XCHAL_EXCCAUSE_NUM + 2: ('DoubleException', 'Double exception'),
    XCHAL_EXCCAUSE_NUM + 3: ('KernelException', 'Unhandled kernel exception'),
    XCHAL_EXCCAUSE_NUM + 4: ('CoprocessorException', 'Coprocessor exception'),
    XCHAL_EXCCAUSE_NUM + 5: ('InterruptWDTTimoutCPU0', 'Interrupt wdt timeout on CPU0'),
    XCHAL_EXCCAUSE_NUM + 6: ('InterruptWDTTimoutCPU1', 'Interrupt wdt timeout on CPU1'),
    XCHAL_EXCCAUSE_NUM + 7: ('CacheError', 'Cache disabled but cached memory region accessed'),
}

def format_extra_data(extra_data):
    result = ""
    if 'crashed_task_handle' in extra_data:
        result += f"Crashed task handle {hex(extra_data['crashed_task_handle'])}\n"
        del extra_data['crashed_task_handle']

    for k, v in extra_data.items():
        if k == 'EXCCAUSE':
            result += f"{k.ljust(16)}{hex(v).ljust(16)}{': '.join(XTENSA_EXCEPTION_CAUSE_DICT[v])}\n"
        else:
            result += f"{k.ljust(16)}{hex(v).ljust(16)}{v}\n"
    return result

def get_tf_coredump_data(coredump_path: str):
    printable = set(string.printable)
    found_str = ""
    with open(coredump_path, "rb") as file:
        b = file.read()
        start = b.find(PREFIX)

        if start < 0:
            return (None, None)

        end = b.find(SUFFIX, start)

        if end < 0:
            return (None, None)

        start += len(PREFIX)
        tf_core_dump_data = b[start:end]

        # based on https://github.com/espressif/esp-coredump/blob/08f92cf36e7a649650a8b722cc96217026fd0174/esp_coredump/corefile/xtensa.py#L120
        # and staring at an ELF file

        # search xtensa extra info behind out core dump data
        extra_info_idx = b.find(EXTRA_INFO_HEADER, end)
        if extra_info_idx < 0 or (len(b) - extra_info_idx < (len(EXTRA_INFO_HEADER) + 2 + 108)):
            return (tf_core_dump_data, None)

        extra_data = {}

        # skip header
        extra_info_idx += len(EXTRA_INFO_HEADER)
        # skip two bytes (probably part of the ELF section header)
        extra_info_idx += 2

        extra_info = b[extra_info_idx:extra_info_idx+108]
        extra_data['crashed_task_handle'] = int.from_bytes(extra_info[:4], byteorder='little')

        for i in range(4, len(extra_info), 8):
            reg = int.from_bytes(extra_info[i:i+4], byteorder='little')
            if reg == 0:
                continue

            value = int.from_bytes(extra_info[i+4:i+8], byteorder='little')

            extra_data[extra_info_reg_name(reg)] = value

        return (tf_core_dump_data, extra_data)

core_dump_path = os.path.join(tempfile.gettempdir(), "tf_coredump.elf")

def get_core_dump_from_debug_report(path):
    with open(path, 'r', encoding='utf-8') as file:
        file_str = file.read()
        core_dump_start_pos = file_str.rfind("___CORE_DUMP_START___\n\n")
        if core_dump_start_pos < 0:
            print("Debug log doesn't contain a core dump")
            sys.exit(-1)

        core_dump_b64 = file_str[core_dump_start_pos + 60:]
        core_dump = base64.b64decode(core_dump_b64)

        with open(core_dump_path, "wb") as f:
            f.write(core_dump)

def download_core_dump(port):
    # The partition table is always at the same offset, so read it first,
    # get the core dump partition offset and size and then read the core dump itself.
    os.system("pio pkg exec esptool.py -- --port {} --chip esp32 --baud 921600 read_flash 0x8000 0x1000 {}".format(port, core_dump_path))

    core_dump_offset = None
    core_dump_size = None

    target = parttool.ParttoolTarget(core_dump_path)
    for p in target.partition_table:
        if p.type == parttool.DATA_TYPE and p.subtype == parttool.SUBTYPES[parttool.DATA_TYPE]['coredump']:
            core_dump_offset = p.offset
            core_dump_size = p.size

    if core_dump_offset is None or core_dump_size is None:
        raise Exception("Failed to get core dump partition offset or size from partition table!")

    # Remove header before ELF file
    core_dump_offset += 20
    core_dump_size -= 20

    os.system("pio pkg exec esptool.py -- --port {} --chip esp32 --baud 921600 read_flash {} {} {}".format(port, core_dump_offset, core_dump_size, core_dump_path))

# use "with ChangedDirectory('/path/to/abc')" instead of "os.chdir('/path/to/abc')"
class ChangedDirectory(object):
    def __init__(self, path):
        self.path = path
        self.previous_path = None

    def __enter__(self):
        self.previous_path = os.getcwd()
        os.chdir(self.path)

    def __exit__(self, type_, value, traceback):
        os.chdir(self.previous_path)

if __name__ == '__main__':
    gdb = find_gdb()
    if gdb is None:
        print("Can't find xtensa-esp32-elf-gdb or pio.")
        print("Make sure that one of them is available in your $PATH.")
        sys.exit(-1)

    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--interactive", action='store_true', help="Don't exit gdb immediately")
    parser.add_argument("-l", "--local-source", action='store_true', help="Don't checkout firmware git, use local copy")
    parser.add_argument("-e", "--elf")

    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("path", nargs='?', default=None)
    group.add_argument("-p", "--port")
    args = parser.parse_args(sys.argv[1:])

    if args.port:
        if len(args.port) < 6:
            port = "/dev/ttyUSB" + args.port
        else:
            port = args.port

    if args.path:
        get_core_dump_from_debug_report(args.path)
    if args.port:
        download_core_dump(port)


    try:
        tf_coredump_json, extra_data = get_tf_coredump_data(core_dump_path)
        if not tf_coredump_json:
            print("Core dump in debug log has no TF coredump info.")
            sys.exit (-1)

        tf_coredump_data = json.loads(tf_coredump_json)
        if extra_data is not None:
            extra_data = format_extra_data(extra_data).replace('\n', '\\n')

        elf_name = tf_coredump_data['firmware_file_name'] + ".elf"
        script_path = os.path.dirname(os.path.realpath(__file__))

        possible_firmware_paths = [
            os.path.join(script_path, "build", elf_name),
            os.path.join(script_path, "..", "..", "warp-charger", "firmwares", elf_name),
            os.path.join(".", elf_name),
        ]

        if args.elf:
            possible_firmware_paths.insert(0, args.elf)

        firmware_path = None
        for path in possible_firmware_paths:
            if os.path.exists(path):
                firmware_path = path


        def run_gdb(repo_dir="../"):
            coredump_py_gdb_cmds = ""
            if os.path.exists("coredump_py_gdb_cmds"):
                with open("coredump_py_gdb_cmds", "r", encoding="utf-8") as f:
                    coredump_py_gdb_cmds = f.read().replace("\n", " ")

            os.system(f"{gdb} " +
                        ("-q --batch " if not args.interactive else "") +
                        "-iex 'set pagination off' " +
                        f"-iex 'directory {repo_dir}' " +
                        f"-iex 'set substitute-path src/ {repo_dir}/software/src' " +
                        f"-iex 'set substitute-path /home/erik/ {os.path.expanduser('~')}' " +
                        "-iex 'set style enabled on' " +
                        "-iex 'set print frame-info source-and-location' " +
                        coredump_py_gdb_cmds +
                        ("-ex 'shell clear' " if args.interactive else "") +
                        "-ex 'echo ================================================================================\n' " +
                        "-ex 'echo In interactive mode:\n' " +
                        "-ex 'echo     - Run \"disassemble /s\" to analyze assembly.\n' " +
                        "-ex 'echo     - Run \"thread apply all bt full\" to print traces of all threads.\n' " +
                        "-ex 'echo\n' " +
                        f"-ex 'echo Crashed firmware {tf_coredump_data['firmware_file_name']}\n' " +
                        (f"-ex 'echo {extra_data}\n' " if extra_data is not None else "")+
                        "-ex 'echo ================================================================================\n' " +
                        "-ex 'echo ============================= Registers at crash ===============================\n' " +
                        "-ex 'echo ================================================================================\n' " +
                        "-ex 'info registers pc ps a0 a1 a2 a3 a4 a5 a6 a7 a8 a9 a10 a11 a12 a13 a14 a15 sar lbeg lend lcount' " +
                        "-ex 'echo ================================================================================\n' " +
                        "-ex 'echo ============================= Backtrace starts here ============================\n' " +
                        "-ex 'echo ================================================================================\n' " +
                        "-ex 'bt full' " +
                        f"{firmware_path} {core_dump_path}")

        if firmware_path:
            if args.local_source:
                run_gdb()
            else:
                with tempfile.TemporaryDirectory(prefix="coredump-git-") as d:
                    os.system(f"git clone --shared --no-checkout {script_path}/.. {d}")
                    with ChangedDirectory(d):
                        os.system(f"git checkout --quiet {tf_coredump_data['firmware_commit_id']}")
                        commit_time = int(subprocess.check_output(['git', 'log', '-1', '--pretty=%at', tf_coredump_data['firmware_commit_id']]))
                        for (dirpath, dirnames, filenames) in os.walk('software/src'):
                            for filename in filenames:
                                os.utime(os.sep.join([dirpath, filename]), (commit_time, commit_time))

                    run_gdb(d)


        else:
            print(f"Firmware {elf_name} not found in any of these places:\n")
            for path in possible_firmware_paths:
                print(f"    {path}")
    finally:
        if os.path.exists(core_dump_path):
            os.remove(core_dump_path)
