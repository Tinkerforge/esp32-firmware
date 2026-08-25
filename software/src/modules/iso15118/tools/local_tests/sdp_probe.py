#!/usr/bin/env python3
"""SDP probe: sends one SDP DiscoveryRequest and prints the response.

./sdp_probe.py --charger 192.168.1.147 [--security 0x10] [--iface enp1s0]

--eval prints shell variable assignments (SECC_LL, SECC_PORT, IFACE) for the
bash based tests. Exit code 1 on timeout.
"""

import argparse
import sys
import time

import common


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--charger", required=True)
    p.add_argument("--iface")
    p.add_argument("--security", default="0x00", help="SDP security byte, 0x00 TLS, 0x10 no TLS")
    p.add_argument("--retries", type=int, default=1)
    p.add_argument("--eval", action="store_true")
    args = p.parse_args()

    iface = args.iface or common.default_iface(args.charger)
    res = None
    for _ in range(args.retries):
        res = common.sdp_request(iface, int(args.security, 0))
        if res is not None:
            break
        time.sleep(1)
    if res is None:
        print("TIMEOUT", file=sys.stderr)
        raise SystemExit(1)

    if args.eval:
        print(f'SECC_LL={res["secc_ll"]}')
        print(f'SECC_PORT={res["port"]}')
        print(f'IFACE={iface}')
    else:
        print(f'from={res["from"]} secc_ll={res["secc_ll"]} port={res["port"]} '
              f'security={res["security"]:#04x} transport={res["transport"]:#04x}')


if __name__ == "__main__":
    main()
