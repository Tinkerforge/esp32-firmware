#!/usr/bin/python3 -u

import os
import csv

for name in ['meter_value_id.csv', 'meter_value_group.csv', 'meter_value_fragment.csv']:
    with open(name, newline='', encoding='utf-8') as f:
        rows = list(csv.reader(f))

    with open(name + '.tmp', 'w', newline='', encoding='utf-8') as f:
        csv.writer(f, lineterminator='\n').writerows(rows)

    os.replace(name + '.tmp', name)
