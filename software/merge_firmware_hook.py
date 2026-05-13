import sys
import glob
import os
import shutil
import subprocess
import traceback
import pathlib
import json
import hashlib
from zlib import crc32

env = None

Import('env')

with open(env.subst(f'$BUILD_DIR{os.sep}metadata.json'), 'r', encoding='utf-8') as f:
    metadata = json.loads(f.read())

product_id = metadata['product_id']
signature_preset = metadata['signature_preset']
firmware_basename = metadata['firmware_basename']

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


def merge_firmware():
    write_firmware_info()

    subprocess.check_call([
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
        '-o', env.subst(f'$BUILD_DIR{os.sep}${{PROGNAME}}_merged.bin'),
        '--target-offset', '0x1000',
        '0x1000', env.subst(f'$BUILD_DIR{os.sep}bootloader.bin'),
        '0x8000', env.subst(f'$BUILD_DIR{os.sep}partitions.bin'),
        '0xd000', env.subst(f'$BUILD_DIR{os.sep}firmware_info.bin'),
        '0xe000', 'boot_app0.bin',
        '0x10000', env.subst(f'$BUILD_DIR{os.sep}${{PROGNAME}}.bin')
    ])


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

add_post_action_bin(f'Merging {env.subst(f"$BUILD_DIR{os.sep}$PROGNAME")}.bin -> {env.subst(f"$BUILD_DIR{os.sep}$PROGNAME")}_merged.bin', merge_firmware)

if len(signature_preset) > 0:
    add_post_action_bin(f'Signing $BUILD_DIR{os.sep}${{PROGNAME}}_merged.bin -> build{os.sep}{firmware_basename}_merged.bin',
                        lambda: subprocess.check_call([
                            'uv',
                            'run',
                            env.subst(f'$PROJECT_DIR{os.sep}signature{os.sep}sign.py'),
                            signature_preset,
                            env.subst(f'$BUILD_DIR{os.sep}${{PROGNAME}}_merged.bin'),
                            f'build{os.sep}{firmware_basename}_merged.bin'
                        ], env=os.environ | {'PYTHONUNBUFFERED': '1'}))

else:
    add_post_action_bin(f'Copying $BUILD_DIR{os.sep}${{PROGNAME}}_merged.bin -> build{os.sep}{firmware_basename}_merged.bin',
                        lambda: shutil.copy2(env.subst(f'$BUILD_DIR{os.sep}${{PROGNAME}}_merged.bin'),
                                             f'build{os.sep}{firmware_basename}_merged.bin'))

add_post_action_bin(f'Symlinking build{os.sep}{firmware_basename}_merged.bin -> build_latest{os.sep}{firmware_basename}_merged.bin',
                    lambda: remove_and_symlink(f'build_latest{os.sep}{product_id}_firmware*_merged.bin',
                                               f'..{os.sep}build{os.sep}{firmware_basename}_merged.bin',
                                               f'build_latest{os.sep}{firmware_basename}_merged.bin'))

add_post_action_bin(f'Symlinking build{os.sep}{firmware_basename}_merged.bin -> build{os.sep}{product_id}_firmware_latest_merged.bin',
                    lambda: symlink(f'{firmware_basename}_merged.bin',
                                    f'build{os.sep}{product_id}_firmware_latest_merged.bin'))

add_post_action_bin(f'Symlinking build{os.sep}{firmware_basename}_merged.bin -> build{os.sep}firmware_latest_merged.bin',
                    lambda: symlink(f'{firmware_basename}_merged.bin',
                                    f'build{os.sep}firmware_latest_merged.bin'))
