import sys
import glob
import os
import shutil
import subprocess
import traceback
import pathlib
import json
import hashlib
import csv
import secure_boot as secure_boot_py
from zlib import crc32

env = None

Import('env')

if 'compiledb' in COMMAND_LINE_TARGETS:
    sys.exit(0)

with open(env.subst(f'$BUILD_DIR{os.sep}metadata.json'), 'r', encoding='utf-8') as f:
    metadata = json.loads(f.read())

product_id = metadata['product_id']
signature_preset = metadata['signature_preset']
firmware_basename = metadata['firmware_basename']
split_esptool_ota = metadata['split_esptool_ota']
secure_boot = metadata['secure_boot']

if sys.platform == 'win32':
    def symlink(src, dst):
        shutil.copy2(os.path.join('build', src), dst)
else:
    symlink = os.symlink


def add_post_action_elf(title, action):
    def wrapper(**kwargs):  # hide potential return value of action
        try:
            action()
        except:
            traceback.print_exc()
            raise

    env.AddPostAction(f'$BUILD_DIR{os.sep}${{PROGNAME}}.elf', env.VerboseAction(wrapper, title))


def add_post_action_bin(title, action):
    def wrapper(**kwargs):  # hide potential return value of action
        try:
            action()
        except:
            traceback.print_exc()
            raise

    env.AddPostAction(f'$BUILD_DIR{os.sep}${{PROGNAME}}.bin', env.VerboseAction(wrapper, title))


def remove_and_symlink(remove_pattern, src_path, dst_path):
    for path in glob.glob(remove_pattern):
        try:
            os.remove(path)
        except FileNotFoundError:
            pass

    return symlink(src_path, dst_path)


def write_firmware_info():
    firmware_info_product_id = metadata['firmware_info_product_id']
    firmware_info_product_name = metadata['firmware_info_product_name']
    version = metadata['version']
    build_timestamp = metadata['build_timestamp']

    print(f'Firmware info: {firmware_info_product_id}, {firmware_info_product_name}, {".".join(version[:3]) + ("-beta." + version[3] if version[3] != "255" else "")}, {build_timestamp:08X}')

    buf = bytearray([0xFF] * 4096)

    # 7121CE12F0126E
    # tink er for ge
    buf[0:7] = bytes.fromhex("7121CE12F0126E")  # magic
    buf[7] = 0x04  # firmware info version, note: a new version has to be backwards compatible
    firmware_info_product_name_bytes = firmware_info_product_name.encode("utf-8")  # max 60 bytes

    if len(firmware_info_product_name_bytes) > 60:
        raise Exception('firmware_info_product_name is longer than 60 bytes')

    buf[8:8 + len(firmware_info_product_name_bytes)] = firmware_info_product_name_bytes
    buf[8 + len(firmware_info_product_name_bytes):68] = bytes(60 - len(firmware_info_product_name_bytes))
    buf[68] = 0x00  # 0 byte to make sure string is terminated. also pads the version, so that the build timestamp will be 4-byte aligned
    buf[69] = int(version[0])
    buf[70] = int(version[1])
    buf[71] = int(version[2])
    buf[72:76] = build_timestamp.to_bytes(4, byteorder='little')
    buf[76] = int(version[3])  # since firmware info version 2
    firmware_info_product_id_bytes = firmware_info_product_id.encode("utf-8")  # max 30 bytes

    if len(firmware_info_product_id_bytes) > 60:
        raise Exception('firmware_info_product_id is longer than 60 bytes')

    application_bytes = pathlib.Path(env.subst(f'$BUILD_DIR{os.sep}${{PROGNAME}}.bin')).read_bytes()
    padded_application_bytes = application_bytes + bytearray([0xFF] * ((((len(application_bytes) + 4095)) & ~4095) - len(application_bytes)))  # pad to multiple of 4096

    buf[77:77 + len(firmware_info_product_id_bytes)] = firmware_info_product_id_bytes  # since firmware info version 3
    buf[77 + len(firmware_info_product_id_bytes):137] = bytes(60 - len(firmware_info_product_id_bytes))
    buf[137] = 0x00  # 0 byte to make sure string is terminated
    buf[138:170] = hashlib.sha256(padded_application_bytes).digest()  # since firmware info version 4
    buf[4092:4096] = crc32(buf[0:4092]).to_bytes(4, byteorder='little')

    pathlib.Path(env.subst(f'$BUILD_DIR{os.sep}firmware_info.bin')).write_bytes(buf)


partitions = {}

with open(env.GetProjectOption('board_build.partitions'), newline='') as csvfile:
    reader = csv.DictReader(csvfile, fieldnames=['Name', 'Type', 'SubType', 'Offset', 'Size', 'Flags'], skipinitialspace=True)
    for row in reader:
        partitions[row['Name']] = row

app_offset = partitions['app0']['Offset']
otadata_offset = partitions['otadata']['Offset']

if split_esptool_ota:
    partition_table_offset = '0xe000'
else:
    partition_table_offset = '0x8000'

merge_cmd_base = [
    'uv',
    'run',
    'pio',
    'pkg',
    'exec',
    '-p', 'tool-esptoolpy',
    '--',
    'esptool',
    '--chip', 'esp32',
    'merge-bin',
    '--target-offset', '0x1000',
]

def merge_firmware_merged():
    global merge_cmd_base
    global partition_table_offset
    global otadata_offset
    global app_offset

    cmd = merge_cmd_base + [
        '-o',                   env.subst(f'$BUILD_DIR{os.sep}${{PROGNAME}}_merged.bin'),
        '0x1000',               env.subst(f'$BUILD_DIR{os.sep}bootloader.bin'),
        partition_table_offset, env.subst(f'$BUILD_DIR{os.sep}partitions.bin'),
        '0xd000',               env.subst(f'$BUILD_DIR{os.sep}firmware_info.bin'),
        otadata_offset,         'boot_app0.bin',
        app_offset,             env.subst(f'$BUILD_DIR{os.sep}${{PROGNAME}}.bin'),
    ]

    subprocess.check_call(cmd)

def merge_firmware_esptool():
    global merge_cmd_base
    global partition_table_offset
    global otadata_offset
    global app_offset

    cmd = merge_cmd_base + [
        '-o',                   env.subst(f'$BUILD_DIR{os.sep}${{PROGNAME}}_esptool.bin'),
        '0x1000',               env.subst(f'$BUILD_DIR{os.sep}bootloader.bin'),
        partition_table_offset, env.subst(f'$BUILD_DIR{os.sep}partitions.bin'),
        otadata_offset,         'boot_app0.bin',
        app_offset,             env.subst(f'$BUILD_DIR{os.sep}${{PROGNAME}}.bin')
    ]

    subprocess.check_call(cmd)

def merge_firmware_esptool_secure():
    global merge_cmd_base
    global partition_table_offset
    global otadata_offset
    global secure_boot

    # Expected format of the custom_secure_boot setting: "<UID/name>,<efuses block3>,<factory MAC>"
    # Example:
    # custom_secure_boot = "8555,0000123456789abcdef0123456789abcdef012345678ab00cdef012345678900,a8:03:2a:12:34:56"

    sb = secure_boot.split(',')
    encryption_target_name = sb[0]
    efuses_block3 = sb[1]
    factory_mac = sb[2]

    f_boot     = env.subst(f'$BUILD_DIR{os.sep}bootloader.bin')
    f_boot_sig = env.subst(f'$BUILD_DIR{os.sep}bootloader_signed.bin')
    f_boot_enc = env.subst(f'$BUILD_DIR{os.sep}bootloader_signed_encrypted.bin')
    secure_boot_py.sign_str(f_boot, f_boot_sig, efuses_block3, factory_mac)
    secure_boot_py.encrypt_str(f_boot_sig, f_boot_enc, '0x1000', efuses_block3, factory_mac)

    f_part     = env.subst(f'$BUILD_DIR{os.sep}partitions.bin')
    f_part_enc = env.subst(f'$BUILD_DIR{os.sep}partitions_encrypted.bin')
    secure_boot_py.encrypt_str(f_part, f_part_enc, partition_table_offset, efuses_block3, factory_mac)

    f_ota     = 'boot_app0.bin'
    f_ota_enc = env.subst(f'$BUILD_DIR{os.sep}boot_app0_encrypted.bin')
    secure_boot_py.encrypt_str(f_ota, f_ota_enc, otadata_offset, efuses_block3, factory_mac)

    f_app     = env.subst(f'$BUILD_DIR{os.sep}${{PROGNAME}}.bin')
    f_app_sig = env.subst(f'$BUILD_DIR{os.sep}${{PROGNAME}}_signed.bin')
    f_app_enc = env.subst(f'$BUILD_DIR{os.sep}${{PROGNAME}}_signed_encrypted.bin')
    secure_boot_py.sign_str(f_app, f_app_sig, efuses_block3, factory_mac)
    secure_boot_py.encrypt_str(f_app_sig, f_app_enc, app_offset, efuses_block3, factory_mac)

    f_out = env.subst(f'$BUILD_DIR{os.sep}${{PROGNAME}}_esptool_secure_{encryption_target_name}.bin')

    cmd = merge_cmd_base + [
        '-o',                   f_out,
        '0x1000',               f_boot_enc,
        partition_table_offset, f_part_enc,
        otadata_offset,         f_ota_enc,
        app_offset,             f_app_enc,
    ]

    subprocess.check_call(cmd)

def merge_firmware_ota():
    global merge_cmd_base
    global app_offset

    cmd = merge_cmd_base + [
        '-o',       env.subst(f'$BUILD_DIR{os.sep}${{PROGNAME}}_ota.bin'),
        '0xd000',   env.subst(f'$BUILD_DIR{os.sep}firmware_info.bin'),
        app_offset, env.subst(f'$BUILD_DIR{os.sep}${{PROGNAME}}.bin'),
    ]

    subprocess.check_call(cmd)


add_post_action_elf('Ensuring build directory exists',
                    lambda: os.makedirs('build', exist_ok=True))

add_post_action_elf('Ensuring build_latest directory exists',
                    lambda: os.makedirs('build_latest', exist_ok=True))

add_post_action_elf(f'Copying $BUILD_DIR{os.sep}${{PROGNAME}}.elf -> build{os.sep}{firmware_basename}.elf',
                    lambda: shutil.copy2(env.subst(f'$BUILD_DIR{os.sep}${{PROGNAME}}.elf'), f'build{os.sep}{firmware_basename}.elf'))

add_post_action_elf(f'Symlinking build{os.sep}{firmware_basename}.elf -> build_latest{os.sep}{firmware_basename}.elf',
                    lambda: remove_and_symlink(f'build_latest{os.sep}{product_id}_firmware*.elf',
                                               f'..{os.sep}build{os.sep}{firmware_basename}.elf',
                                               f'build_latest{os.sep}{firmware_basename}.elf'))

add_post_action_elf(f'Symlinking build{os.sep}{firmware_basename}.elf -> build{os.sep}{product_id}_firmware_latest.elf',
                    lambda: symlink(f'{firmware_basename}.elf',
                                    f'build{os.sep}{product_id}_firmware_latest.elf'))

add_post_action_elf(f'Symlinking build{os.sep}{firmware_basename}.elf -> build{os.sep}firmware_latest.elf',
                    lambda: symlink(f'{firmware_basename}.elf',
                                    f'build{os.sep}firmware_latest.elf'))

add_post_action_bin(f'Writing firmware info', write_firmware_info)

if split_esptool_ota:
    add_post_action_bin(f'Merging {env.subst(f"$BUILD_DIR{os.sep}$PROGNAME")}.bin -> {env.subst(f"$BUILD_DIR{os.sep}$PROGNAME")}_esptool.bin', merge_firmware_esptool)
    add_post_action_bin(f'Merging {env.subst(f"$BUILD_DIR{os.sep}$PROGNAME")}.bin -> {env.subst(f"$BUILD_DIR{os.sep}$PROGNAME")}_ota.bin', merge_firmware_ota)
    if len(secure_boot) > 0:
        encryption_target_name = secure_boot.split(',')[0]
        add_post_action_bin(f'Merging {env.subst(f"$BUILD_DIR{os.sep}$PROGNAME")}.bin -> {env.subst(f"$BUILD_DIR{os.sep}$PROGNAME")}_esptool_secure_{encryption_target_name}.bin', merge_firmware_esptool_secure)
else:
    add_post_action_bin(f'Merging {env.subst(f"$BUILD_DIR{os.sep}$PROGNAME")}.bin -> {env.subst(f"$BUILD_DIR{os.sep}$PROGNAME")}_merged.bin', merge_firmware_merged)

if split_esptool_ota:
    unsigned_suffixes = ['esptool']
    signed_suffixes = ['ota']
else:
    unsigned_suffixes = []
    signed_suffixes = ['merged']

if len(signature_preset) == 0:
    unsigned_suffixes += signed_suffixes
    signed_suffixes = []

all_suffixes = unsigned_suffixes + signed_suffixes

for suffix in signed_suffixes:
    add_post_action_bin(f'Signing $BUILD_DIR{os.sep}${{PROGNAME}}_{suffix}.bin -> build{os.sep}{firmware_basename}_{suffix}.bin',
                        lambda suffix=suffix: subprocess.check_call([
                            'uv',
                            'run',
                            env.subst(f'$PROJECT_DIR{os.sep}signature{os.sep}sign.py'),
                            signature_preset,
                            env.subst(f'$BUILD_DIR{os.sep}${{PROGNAME}}_{suffix}.bin'),
                            f'build{os.sep}{firmware_basename}_{suffix}.bin'
                        ], env=os.environ | {'PYTHONUNBUFFERED': '1'}))

for suffix in unsigned_suffixes:
    add_post_action_bin(f'Copying $BUILD_DIR{os.sep}${{PROGNAME}}_{suffix}.bin -> build{os.sep}{firmware_basename}_{suffix}.bin',
                        lambda suffix=suffix: shutil.copy2(env.subst(f'$BUILD_DIR{os.sep}${{PROGNAME}}_{suffix}.bin'),
                                            f'build{os.sep}{firmware_basename}_{suffix}.bin'))

for suffix in all_suffixes:
    add_post_action_bin(f'Symlinking build{os.sep}{firmware_basename}_{suffix}.bin -> build_latest{os.sep}{firmware_basename}_{suffix}.bin',
                        lambda suffix=suffix: remove_and_symlink(f'build_latest{os.sep}{product_id}_firmware*_{suffix}.bin',
                                                                 f'..{os.sep}build{os.sep}{firmware_basename}_{suffix}.bin',
                                                                 f'build_latest{os.sep}{firmware_basename}_{suffix}.bin'))

    add_post_action_bin(f'Symlinking build{os.sep}{firmware_basename}_{suffix}.bin -> build{os.sep}{product_id}_firmware_latest_{suffix}.bin',
                        lambda suffix=suffix: symlink(f'{firmware_basename}_{suffix}.bin',
                                                      f'build{os.sep}{product_id}_firmware_latest_{suffix}.bin'))

    add_post_action_bin(f'Symlinking build{os.sep}{firmware_basename}_{suffix}.bin -> build{os.sep}firmware_latest_{suffix}.bin',
                        lambda suffix=suffix: symlink(f'{firmware_basename}_{suffix}.bin',
                                                      f'build{os.sep}firmware_latest_{suffix}.bin'))
