#!/usr/bin/env python3

import struct
import sys

PIB_OFFSET_BASE = 0x3C0
PRESCALER_OFFSET = 0x0F28 + PIB_OFFSET_BASE   # 0x12E8
PRESCALER_COUNT = 578
AFE_GAIN_OFFSET = 0x1471 + PIB_OFFSET_BASE    # 0x1831
DATA_CHECKSUM_OFFSET = 0x374
HEADER_CHECKSUM_OFFSET = 0x3BC
DEVICE_KEY_OFFSET = 0x3D2
DEVICE_KEY_LEN = 16

IGNORED_REGIONS = [
    (DATA_CHECKSUM_OFFSET, 4, "data checksum"),
    (HEADER_CHECKSUM_OFFSET, 4, "header checksum"),
    (DEVICE_KEY_OFFSET, DEVICE_KEY_LEN, "device security key (NMK/DAK)"),
]


def print(*args):
    pass


def compute_checksum(pib, word_start, word_end):
    xor_sum = 0
    for i in range(word_start, word_end):
        xor_sum ^= struct.unpack_from("<I", pib, i * 4)[0]
    return (0xFFFFFFFF - xor_sum) & 0xFFFFFFFF


def check_internal_checksums(pib):
    data_ok = compute_checksum(pib, 240, len(pib) // 4) == \
        struct.unpack_from("<I", pib, DATA_CHECKSUM_OFFSET)[0]
    hdr_ok = compute_checksum(pib, 216, 239) == \
        struct.unpack_from("<I", pib, HEADER_CHECKSUM_OFFSET)[0]
    return data_ok, hdr_ok


def is_ignored(offset):
    for start, length, _ in IGNORED_REGIONS:
        if start <= offset < start + length:
            return True
    return False


def compare(path_a, path_b):
    a = open(path_a, "rb").read()
    b = open(path_b, "rb").read()

    print(f"A: {path_a} ({len(a)} bytes)")
    print(f"B: {path_b} ({len(b)} bytes)\n")

    return compare_data(a, b)


def compare_data(a, b):
    if len(a) != len(b):
        print("FILES DIFFER IN SIZE -> not comparable")
        return False

    for name, p in (("A", a), ("B", b)):
        d_ok, h_ok = check_internal_checksums(p)
        print(f"  {name} checksums valid: data={'OK' if d_ok else 'BAD'} "
              f"header={'OK' if h_ok else 'BAD'}")
    print()

    relevant_diffs = [i for i in range(len(a))
                      if a[i] != b[i] and not is_ignored(i)]

    print("  Ignored fields (expected to differ):")
    for start, length, label in IGNORED_REGIONS:
        va = a[start:start + length].hex()
        vb = b[start:start + length].hex()
        flag = "changed" if va != vb else "same"
        print(f"    0x{start:04X} {label:32s} {flag}")
    print()

    pre_a = a[PRESCALER_OFFSET:PRESCALER_OFFSET + PRESCALER_COUNT]
    pre_b = b[PRESCALER_OFFSET:PRESCALER_OFFSET + PRESCALER_COUNT]
    afe_a = struct.unpack_from("<i", a, AFE_GAIN_OFFSET)[0]
    afe_b = struct.unpack_from("<i", b, AFE_GAIN_OFFSET)[0]
    print("  Calibration payload:")
    print(f"    Prescaler table ({PRESCALER_COUNT} bytes): "
          f"{'identical' if pre_a == pre_b else 'DIFFERENT'}")
    print(f"    AFE gain: A={afe_a}  B={afe_b}  "
          f"{'identical' if afe_a == afe_b else 'DIFFERENT'}")
    print()

    if not relevant_diffs:
        print("RESULT: Content is IDENTICAL")
        return True

    print(f"RESULT: Content DIFFERS in {len(relevant_diffs)} byte(s):")
    start = prev = relevant_diffs[0]
    for off in relevant_diffs[1:] + [None]:
        if off == prev + 1:
            prev = off
            continue
        print(f"    0x{start:04X}-0x{prev:04X} ({prev - start + 1} bytes)")
        start = prev = off
    return False


def main():
    if len(sys.argv) != 3:
        print("usage: pib_compare.py <file_a.pib> <file_b.pib>")
        sys.exit(2)
    same = compare(sys.argv[1], sys.argv[2])
    sys.exit(0 if same else 1)


if __name__ == "__main__":
    main()
