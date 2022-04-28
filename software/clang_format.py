#!/usr/bin/env python3

import os

for root, dirs, files in sorted(os.walk('src')):
    if "bindings" in root:
        continue

    for name in sorted(files):
        if not name.endswith('.c') and not name.endswith('.cpp') and not name.endswith('.h'):
            continue

        if ".embedded" in name:
            continue

        path = os.path.join(root, name)

        print('>>>>>>', path)

        os.system('clang-format-12 --style=file --Wno-error=unknown -i "{0}"'.format(path))
