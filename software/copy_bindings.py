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

def remove_files(target_dir):
    for name in sorted(os.listdir(target_dir)):
        target_path = os.path.join(target_dir, name)

        if verbose:
            print('remove', target_path)

        os.remove(target_path)

def copy_files(source_dir, target_dir, exclude_pattern=None, include_pattern=None, patch_include=False):
    for name in sorted(os.listdir(source_dir)):
        if exclude_pattern != None and re.match(exclude_pattern, name) != None:
            continue

        if include_pattern != None and re.match(include_pattern, name) == None:
            continue

        source_path = os.path.join(source_dir, name)
        target_path = os.path.join(target_dir, name)

        if verbose:
            print('copy', source_path, target_path)

        with open(source_path, 'rb') as f:
            data = f.read()

        if patch_include:
            data = data.replace(b'#include "../bindings/', b'#include "bindings/')

        with open(target_path, 'wb') as f:
            f.write(data)

remove_files(bindings_target_dir)
remove_files(net_arduino_esp32_target_dir)
remove_files(hal_arduino_esp32_brick_target_dir)
remove_files(hal_arduino_esp32_ethernet_brick_target_dir)

copy_files(generators_uc_dir, bindings_target_dir, include_pattern=r'^.*\.(h|c|inc)$')
copy_files(os.path.join(generators_uc_dir, 'bindings'), bindings_target_dir, exclude_pattern=r'^__released_files__$')
copy_files(os.path.join(generators_uc_dir, 'net_arduino_esp32'), net_arduino_esp32_target_dir, include_pattern=r'^.*\.(h|c|cpp)$')
copy_files(os.path.join(generators_uc_dir, 'hal_arduino_esp32_brick'), hal_arduino_esp32_brick_target_dir, include_pattern=r'^.*\.(h|c|cpp)$', patch_include=True)
copy_files(os.path.join(generators_uc_dir, 'hal_arduino_esp32_ethernet_brick'), hal_arduino_esp32_ethernet_brick_target_dir, include_pattern=r'^.*\.(h|c|cpp)$', patch_include=True)
