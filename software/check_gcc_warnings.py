from pathlib import Path
import re
import subprocess

all_warnings_text = subprocess.run(
    "uv run pio pkg exec xtensa-esp32-elf-gcc -- -Q --help=warnings".split(" "),
    check=True,
    capture_output=True,
    text=True).stdout

enabled_warnings = Path("src/gcc_warnings.h").read_text()

enabled_warnings = re.findall(r'GCC diagnostic (?:error|ignored) "([^"]+)"', enabled_warnings)
enabled_warnings = [x.split("=", maxsplit=1)[0] for x in enabled_warnings]

all_warnings = {}

ignored = [
    "-Wc++-compat",
    "-Wc++0x-compat",
    "-Wc++11-compat",
    "-Wc++14-compat",
    "-Wc++17-compat",
    "-Wc++1z-compat",
    "-Wc++2a-compat",
    "-Wc11-c23-compat",
    "-Wc90-c99-compat",
    "-Wc99-c11-compat",
    "-Wc++11-extensions",
    "-Wc++14-extensions",
    "-Wc++17-extensions",
    "-Wc++23-extensions",
    "-Wc++26-extensions",
    "-W",

    # Those are inverted forms of other warnings
    "-Wno-alloc-size-larger-than",
    "-Wno-alloca-larger-than",
    "-Wno-frame-larger-than",
    "-Wno-larger-than",
    "-Wno-stack-usage",
    "-Wno-vla-larger-than",

    # -Walloc-size-larger-than=‘PTRDIFF_MAX’ is enabled by default
    "-Walloc-size-larger-than=<bytes>",

    # Enabled in gcc_warnings, but syntax in --help=warnings is broken
    "-Wformat-overflow<0,2>",

    # We enable -Wshadow, which is stronger
    "-Wshadow-compatible-local",
    "-Wshadow-local",

]

for l in all_warnings_text.splitlines():
    l = l.strip()
    if not l.startswith("-W"):
        continue

    if " " in l:
        opt, doc = l.split(" ", maxsplit=1)
    else:
        opt = l
        doc = ""

    # space before C++ to exclude ObjC++
    if "available" in doc and " C++" not in doc:
        continue

    if opt in ignored:
        continue

    all_warnings[opt.strip()] = doc.strip()

for k, v in all_warnings.items():
    if k.split("=",maxsplit=1)[0] in enabled_warnings:
        continue

    print(k, v)
