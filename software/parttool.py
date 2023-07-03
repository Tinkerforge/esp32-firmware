#!/usr/bin/env python

# Stripped down parttool.py from here: https://github.com/espressif/esp-idf/blob/v4.4.5/components/partition_table/parttool.py

#
# parttool is used to perform partition level operations - reading,
# writing, erasing and getting info about the partition.
#
# Copyright 2018 Espressif Systems (Shanghai) PTE LTD
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http:#www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
from __future__ import division, print_function, unicode_literals

import sys

# ESP32 partition table generation tool
#
# Converts partition tables to/from CSV and binary formats.
#
# See https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/partition-tables.html
# for explanation of partition table structure and uses.
#
# SPDX-FileCopyrightText: 2016-2021 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Apache-2.0

import binascii
import hashlib
import struct

MD5_PARTITION_BEGIN = b'\xEB\xEB' + b'\xFF' * 14  # The first 2 bytes are like magic numbers for MD5 sum

MIN_PARTITION_SUBTYPE_APP_OTA = 0x10
NUM_PARTITION_SUBTYPE_APP_OTA = 16

APP_TYPE = 0x00
DATA_TYPE = 0x01

TYPES = {
    'app': APP_TYPE,
    'data': DATA_TYPE,
}


# Keep this map in sync with esp_partition_subtype_t enum in esp_partition.h
SUBTYPES = {
    APP_TYPE: {
        'factory': 0x00,
        'test': 0x20,
    },
    DATA_TYPE: {
        'ota': 0x00,
        'phy': 0x01,
        'nvs': 0x02,
        'coredump': 0x03,
        'nvs_keys': 0x04,
        'efuse': 0x05,
        'undefined': 0x06,
        'esphttpd': 0x80,
        'fat': 0x81,
        'spiffs': 0x82,
    },
}

md5sum = True

def from_binary(b):
    md5 = hashlib.md5()
    result = []
    for o in range(0,len(b),32):
        data = b[o:o + 32]
        if len(data) != 32:
            raise Exception('Partition table length must be a multiple of 32 bytes')
        if data == b'\xFF' * 32:
            return result  # got end marker
        if md5sum and data[:2] == MD5_PARTITION_BEGIN[:2]:  # check only the magic number part
            if data[16:] == md5.digest():
                continue  # the next iteration will check for the end marker
            else:
                raise Exception("MD5 checksums don't match! (computed: 0x%s, parsed: 0x%s)" % (md5.hexdigest(), binascii.hexlify(data[16:])))
        else:
            md5.update(data)
        result.append(PartitionDefinition.from_binary(data))
    raise Exception('Partition table is missing an end-of-table marker')

class PartitionDefinition(object):
    MAGIC_BYTES = b'\xAA\x50'

    # dictionary maps flag name (as used in CSV flags list, property name)
    # to bit set in flags words in binary format
    FLAGS = {
        'encrypted': 0
    }

    # add subtypes for the 16 OTA slot values ("ota_XX, etc.")
    for ota_slot in range(NUM_PARTITION_SUBTYPE_APP_OTA):
        SUBTYPES[TYPES['app']]['ota_%d' % ota_slot] = MIN_PARTITION_SUBTYPE_APP_OTA + ota_slot

    def __init__(self):
        self.name = ''
        self.type = None
        self.subtype = None
        self.offset = None
        self.size = None
        self.encrypted = False

    def __str__(self):
        return "Partition '%s' %d/%d @ 0x%x size 0x%x encrypted %s" % (self.name, self.type, self.subtype, self.offset or -1, self.size or -1, "yes" if self.encrypted else "no")

    STRUCT_FORMAT = b'<2sBBLL16sL'

    @classmethod
    def from_binary(cls, b):
        if len(b) != 32:
            raise Exception('Partition definition length must be exactly 32 bytes. Got %d bytes.' % len(b))
        res = cls()
        (magic, res.type, res.subtype, res.offset,
         res.size, res.name, flags) = struct.unpack(cls.STRUCT_FORMAT, b)
        if b'\x00' in res.name:  # strip null byte padding from name string
            res.name = res.name[:res.name.index(b'\x00')]
        res.name = res.name.decode()
        if magic != cls.MAGIC_BYTES:
            raise Exception('Invalid magic bytes (%r) for partition definition' % magic)
        for flag,bit in cls.FLAGS.items():
            if flags & (1 << bit):
                setattr(res, flag, True)
                flags &= ~(1 << bit)
        if flags != 0:
            print('WARNING: Partition definition had unknown flag(s) 0x%08x. Newer binary format?' % flags)
        return res

class ParttoolTarget():
    def __init__(self, partition_table_file):

        partition_table = None
        with open(partition_table_file, 'rb') as f:
            input_is_binary = (f.read(2) == PartitionDefinition.MAGIC_BYTES)
            f.seek(0)
            if not input_is_binary:
                raise Exception("Input is not a binary partition table!")

            partition_table = from_binary(f.read())

        self.partition_table = partition_table

def main():
    target = ParttoolTarget(sys.argv[1])
    partitions = target.partition_table

    for p in partitions:
        print(p)

if __name__ == '__main__':
    main()
