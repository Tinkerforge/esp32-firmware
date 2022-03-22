#!/usr/bin/env python3

import os

for root, dirs, files in sorted(os.walk('src')):
    for name in sorted(files):
        if not name.endswith('.c') and not name.endswith('.cpp') and not name.endswith('.h'):
            continue

        path = os.path.join(root, name)

        print('>>>>>>', path)

        os.system('clang-format-12 --style=file -i "{0}"'.format(path))
