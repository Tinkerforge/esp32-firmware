#!/usr/bin/python3 -u

import os
import re
import sys
import argparse
import socket
import tinkerforge_util as tfutil  # sudo apt install python3-tinkerforge-util

BASE58 = '123456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ'

QR_CODE_FORMAT = 'W288,29,5,2,M,8,4,{0},0\r'
QR_CODE_LENGTH = 49

SSID_PLACEHOLDER = b'esp32-XXXXXX'

PASSPHRASE_PLACEHOLDER = b'ZZZZ-ZZZZ-ZZZZ-ZZZZ'

COPIES_FORMAT = '^C{0}\r'


def print_esp32_label(ssid, passphrase, copies, stdout):
    # check SSID
    if re.match('^(esp32|warp|warp2|warp3|wem)-[{0}]{{3,6}}$'.format(BASE58), ssid) == None:
        raise Exception('Invalid SSID: {0}'.format(ssid))

    # check passphrase
    if re.match('^[{0}]{{4}}-[{0}]{{4}}-[{0}]{{4}}-[{0}]{{4}}$'.format(BASE58), passphrase) == None:
        raise Exception('Invalid passphrase: {0}'.format(passphrase))

    # check copies
    if copies < 1 or copies > 5:
        raise Exception('Invalid copies: {0}'.format(copies))

    # read EZPL file
    with open(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'esp32.prn'), 'rb') as f:
        data = f.read()

    if data.find(b'^H13\r') < 0:
        raise Exception('EZPL file is using wrong darkness setting')

    # patch QR code
    qr_code_command = QR_CODE_FORMAT.format(QR_CODE_LENGTH).encode('ascii')

    if data.find(qr_code_command) < 0:
        raise Exception('QR code command missing in EZPL file')

    data = data.replace(qr_code_command, QR_CODE_FORMAT.format(QR_CODE_LENGTH - len(SSID_PLACEHOLDER) + len(ssid)).encode('ascii'))

    # patch SSID
    if data.find(SSID_PLACEHOLDER) < 0:
        raise Exception('SSID placeholder missing in EZPL file')

    data = data.replace(SSID_PLACEHOLDER, ssid.encode('ascii'))

    # patch passphrase
    if data.find(PASSPHRASE_PLACEHOLDER) < 0:
        raise Exception('Passphrase placeholder missing in EZPL file')

    data = data.replace(PASSPHRASE_PLACEHOLDER, passphrase.encode('ascii'))

    # patch copies
    copies_command = COPIES_FORMAT.format(1).encode('ascii')

    if data.find(copies_command) < 0:
        raise Exception('Copies command missing in EZPL file')

    data = data.replace(copies_command, COPIES_FORMAT.format(copies).encode('ascii'))

    # print label
    if stdout:
        sys.stdout.buffer.write(data)
        sys.stdout.buffer.flush()
    else:
        with socket.create_connection((tfutil.get_tf_printer_host('esp32'), 9100)) as s:
            s.send(data)


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument('ssid')
    parser.add_argument('passphrase')
    parser.add_argument('-c', '--copies', type=int, default=1)
    parser.add_argument('-s', '--stdout', action='store_true')

    args = parser.parse_args()

    print_esp32_label(args.ssid, args.passphrase, args.copies, args.stdout)


if __name__ == '__main__':
    main()
