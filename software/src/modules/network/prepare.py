import csv
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util

unsafe_ports = set()

with open('unsafe_ports.csv', newline='') as csvfile:
    reader = csv.reader(csvfile, delimiter=',', quotechar='"')
    next(reader)  # skip header
    for row in reader:
        for p in row:
            if len(p) > 0:
                unsafe_ports.add(int(p))

cpp = tfutil.specialize_template("unsafe_ports.cpp.template", None, {
    "{{{data}}}": ",\n    ".join([str(p) for p in sorted(unsafe_ports)]),
    "{{{length}}}": str(len(unsafe_ports))
})

util.write_generated_file("generated/unsafe_ports.cpp", cpp)
