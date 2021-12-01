# This file is duplicated for every module that embeds a firmware.
# When editing this file, keep the copies in sync!
import os
import re
import sys
import gzip
import io

with open("recovery.html", "rb") as f:
    compressed = gzip.compress(f.read())

bin_file = io.BytesIO(compressed)

with open("recovery_page.cpp", "w") as cpp:
    cpp.write("extern const char recovery_page[] = {\n")
    written = 0
    b = bin_file.read(12)
    while len(b) != 0:
        # read first to prevent trailing , after last byte
        next_b = bin_file.read(12)
        cpp.write("    " + ", ".join(["0x{:02x}".format(x) for x in b]) + (",\n" if len(next_b) != 0 else "\n"))
        written += len(b)
        b = next_b

    cpp.write("};\n")

with open("recovery_page.h", "w") as h:
    h.write("extern const char recovery_page[];\n")
    h.write("const unsigned int recovery_page_len = {};\n".format(written))
