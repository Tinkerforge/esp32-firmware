#!/usr/bin/env python3

import os
import sys
import re

verbose = '-v' in sys.argv[1:] or '--verbose' in sys.argv[1:]
esp32_software_dir = os.path.dirname(os.path.realpath(__file__))
generators_uc_dir = os.path.realpath(os.path.join(esp32_software_dir, '..', '..', 'generators', 'uc'))
bindings_target_dir = os.path.join(esp32_software_dir, 'src', 'bindings')
net_arduino_esp32_target_dir = os.path.join(esp32_software_dir, 'src', 'net_arduino_esp32')
hal_arduino_esp32_brick_target_dir = os.path.join(esp32_software_dir, 'modules', 'backend', 'esp32_brick', 'hal_arduino_esp32_brick')
hal_arduino_esp32_ethernet_brick_target_dir = os.path.join(esp32_software_dir, 'modules', 'backend', 'esp32_ethernet_brick', 'hal_arduino_esp32_ethernet_brick')

def copy_files(source_dir, target_dir, exclude_pattern=None, include_pattern=None, patch_include=False, ignore_header_diff=False):
    source_names = []

    for name in sorted(os.listdir(source_dir)):
        if exclude_pattern != None and re.match(exclude_pattern, name) != None:
            continue

        if include_pattern != None and re.match(include_pattern, name) == None:
            continue

        source_names.append(name)

    for name in sorted(os.listdir(target_dir)):
        if name in source_names:
            continue

        if exclude_pattern != None and re.match(exclude_pattern, name) != None:
            continue

        if include_pattern != None and re.match(include_pattern, name) == None:
            continue


        target_path = os.path.join(target_dir, name)

        if verbose:
            print('remove', target_path)

        os.remove(target_path)

    for name in source_names:
        source_path = os.path.join(source_dir, name)
        target_path = os.path.join(target_dir, name)

        if ignore_header_diff and os.path.exists(target_path):
            with open(source_path, 'r') as f:
                source_lines = list(f.readlines())

            with open(target_path, 'r') as f:
                target_lines = list(f.readlines())

            if len(source_lines) == len(target_lines):
                marker = ' * This file was automatically generated on '

                for source_line, target_line in zip(source_lines, target_lines):
                    if source_line != target_line:
                        if source_line.startswith(marker) and target_line.startswith(marker):
                            continue

                        break
                else:
                    if verbose:
                        print('skipping', source_path)

                    continue

        if verbose:
            print('copy', source_path, target_path)

        with open(source_path, 'rb') as f:
            data = f.read()

        if patch_include:
            data = data.replace(b'#include "../bindings/', b'#include "bindings/')

        with open(target_path, 'wb') as f:
            f.write(data)

copy_files(generators_uc_dir, bindings_target_dir, include_pattern=r'^.*\.(h|c)$', exclude_pattern=r'^(brick(let)?_.*\.(h|c)|display_names.c)$')
copy_files(os.path.join(generators_uc_dir, 'bindings'), bindings_target_dir, include_pattern=r'^(brick(let)?_.*\.(h|c)|display_names.c)$', exclude_pattern=r'^bricklet_stream_test\.(h|c)$', ignore_header_diff=True)
copy_files(os.path.join(generators_uc_dir, 'net_arduino_esp32'), net_arduino_esp32_target_dir, include_pattern=r'^.*\.(h|c|cpp)$')
copy_files(os.path.join(generators_uc_dir, 'hal_arduino_esp32_brick'), hal_arduino_esp32_brick_target_dir, include_pattern=r'^.*\.(h|c|cpp)$', patch_include=True)
copy_files(os.path.join(generators_uc_dir, 'hal_arduino_esp32_ethernet_brick'), hal_arduino_esp32_ethernet_brick_target_dir, include_pattern=r'^.*\.(h|c|cpp)$', patch_include=True)
