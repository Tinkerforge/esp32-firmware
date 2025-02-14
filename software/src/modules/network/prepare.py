import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

import csv

unsafe_ports = set()

with open('unsafe_ports.csv', newline='') as csvfile:
    reader = csv.reader(csvfile, delimiter=',', quotechar='"')
    next(reader) # skip header
    for row in reader:
        for p in row:
            if len(p) > 0:
                unsafe_ports.add(int(p))

tfutil.specialize_template("unsafe_ports.cpp.template", "unsafe_ports.cpp",{
        "{{{data}}}": ",\n    ".join([str(p) for p in sorted(unsafe_ports)]),
        "{{{length}}}": str(len(unsafe_ports))
    })
