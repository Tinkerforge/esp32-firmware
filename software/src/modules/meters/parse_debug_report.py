#!/usr/bin/env -S python3 -u

import json
import argparse
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software.src.modules.meters.meter_value_id import MeterValueID


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('filename')

    args = parser.parse_args()

    with open(args.filename, 'r') as f:
        skip = True
        lines = []

        for line in f.readlines():
            if skip:
                if len(line.strip()) == 0:
                    skip = False

                continue
            elif len(line.strip()) == 0:
                break
            else:
                lines.append(line)

    report = json.loads(''.join(lines))

    for slot in range(7):
        config_key = f'meters/{slot}/config'
        value_ids_key = f'meters/{slot}/value_ids'
        values_key = f'meters/{slot}/values'

        config = report.get(config_key)
        value_ids = report.get(value_ids_key)
        values = report.get(values_key)

        if config == None:
            continue

        if len(value_ids) == 0:
            continue

        display_name = ''

        if config[1] != None:
            display_name = config[1].get('display_name')

        max_length = max([len(MeterValueID(value_id).name) for value_id in value_ids])

        print(f'\nMeter {slot}: {display_name}')

        for i, value_id in enumerate(value_ids):
            value_name = MeterValueID(value_id).name

            print(f'  {value_name}:{" " * (max_length - len(value_name))} {values[i]}')


if __name__ == '__main__':
    main()

