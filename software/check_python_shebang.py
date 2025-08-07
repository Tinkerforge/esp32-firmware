#!/usr/bin/env -S python3 -u

import os
import subprocess

for root, dirs, files in os.walk('.'):
    for name in files:
        path = os.path.join(root, name)

        if path.startswith('./.pio/') or \
           path.startswith('./build/') or \
           path.startswith('./build_latest/') or \
           path.startswith('./packages/') or \
           path.startswith('./web/node_modules/') or \
           path.startswith('./web/src_tfpp/') or \
           '/__pycache__/' in path:
            continue

        file_type = subprocess.check_output(['file', '-b', path], encoding='utf-8').strip()

        if not file_type.startswith('Python script'):
            continue

        with open(path, 'r') as f:
            first_line = f.readlines()[0].strip()

        st_mode = os.stat(path).st_mode

        if path.endswith('/prepare.py'):
            expected_first_line_prefix = ''
            expected_st_mode = 0o100664
        else:
            expected_first_line_prefix = '#!/usr/bin/env -S python3 -u'
            expected_st_mode = 0o100775

        if not first_line.startswith(expected_first_line_prefix) or st_mode != expected_st_mode:
            print(path)
            print(' ', first_line.startswith(expected_first_line_prefix), first_line)
            print(' ', st_mode == expected_st_mode, f'{st_mode:o}')
            print()
