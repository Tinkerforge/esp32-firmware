#!/usr/bin/python3 -u

import urllib.request
import json
import argparse
from datetime import datetime
import time

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('host')
    parser.add_argument('meter', type=int)
    parser.add_argument('value', type=float)
    parser.add_argument('--interval', type=float, default='0.2')
    parser.add_argument('--timeout', type=float, default='0.5')

    args = parser.parse_args()
    counter = 0

    while True:
        start = time.time()
        request = urllib.request.Request(f'http://{args.host}/meters/{args.meter}/values_update',
                                         data=json.dumps([args.value]).encode('utf-8'),
                                         method='PUT',
                                         headers={'Content-Type': 'application/json'})
        try:
            with urllib.request.urlopen(request, timeout=args.timeout) as f:
                response = f.read().decode('utf-8')

                if len(response) > 0:
                    print(f'{datetime.now().isoformat()} [{counter}] response: {response}')
        except Exception as e:
            print(f'{datetime.now().isoformat()} [{counter}] error: {e}')

        end = time.time()
        duration = end - start

        if duration < args.interval:
            time.sleep(args.interval - duration)

        counter += 1

if __name__ == '__main__':
    main()
