import sys
import glob
import os
import shutil
import subprocess
import traceback

env = None

Import('env')

with open(env.subst(f'$BUILD_DIR{os.sep}firmware_basename'), 'r', encoding='utf-8') as f:
    firmware_basename = f.read().strip()

product_id = env.GetProjectOption('custom_product_id')
signature_preset = env.GetProjectOption('custom_signature_preset')

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

add_post_action_bin(f'Merging {env.subst(f"$BUILD_DIR{os.sep}$PROGNAME")}.bin -> {env.subst(f"$BUILD_DIR{os.sep}$PROGNAME")}_merged.bin',
                    lambda: subprocess.check_call([
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
                    ]))

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
