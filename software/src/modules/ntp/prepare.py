#!/usr/bin/env python3

import sys
import argparse
import json
import os
import datetime
from collections import OrderedDict

import sys
import importlib.util
import importlib.machinery

software_dir = os.path.realpath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))

def create_software_module():
    software_spec = importlib.util.spec_from_file_location('software', os.path.join(software_dir, '__init__.py'))
    software_module = importlib.util.module_from_spec(software_spec)

    software_spec.loader.exec_module(software_module)

    sys.modules['software'] = software_module

if 'software' not in sys.modules:
    create_software_module()

from software import util

ZONES_DIR = "/usr/share/zoneinfo/"

AREAS = ["Africa", "America", "Antarctica", "Arctic", "Asia", "Atlantic", "Australia", "Etc", "Europe", "Indian", "Pacific"]

def get_tz_string(timezone):
    data = open(ZONES_DIR + timezone, "rb").read().split(b"\n")[-2]
    return data.decode("utf-8")

def make_timezones_dict():
    result = OrderedDict()

    for area in AREAS:
        for root, dirs, files in os.walk(os.path.join(ZONES_DIR, area), topdown=True):
            for name in sorted(files):
                if not name[0].isupper():
                    continue

                abspath = os.path.join(root, name)
                timezone = os.path.relpath(abspath, ZONES_DIR)
                try:
                    with open(abspath, 'rb') as f:
                        content = f.read()

                    result[timezone] = content.split(b'\n')[-2].decode('utf-8')
                except:
                    print("Can't parse {}. Skipping.".format(abspath))

    return result

timezone_init_template = """static const char * const {table_name}_keys[] = {{
    {table_keys}
}};

static const TableValue {table_name}_vals[] = {{
    {table_vals}
}};

{table_modifier}const Table {table_name} = {{{table_name}_keys, {table_name}_vals, {len}}};"""
table_val_template = "{{{is_leaf}, {{{value}}}}},"

inits = []

def generate(name, nested_dict):
    table_keys = []
    table_vals = []

    for k, v in sorted(nested_dict.items()):
        table_keys.append('"{}"'.format(k))

        if isinstance(v, dict):
            generate(k, v)
            table_vals.append(table_val_template.format(is_leaf="false", value=".sub_table=&{}".format(k)))
        else:
            table_vals.append(table_val_template.format(is_leaf="true", value='"{}"'.format(v)))

    inits.append(timezone_init_template.format(table_name=name,
                                               table_keys=',\n    '.join(table_keys),
                                               table_vals='\n    '.join(table_vals),
                                               len=len(table_keys),
                                               table_modifier="" if name == "global" else "static "))

if not os.path.isdir(ZONES_DIR):
    sys.exit(0)

with open("timezones.c", "r") as f:
    f.readline()
    generated_db_version = f.readline().split(';')[0].strip()

try:
    with open(os.path.join(ZONES_DIR, "tzdata.zi"), "r") as f:
        installed_db_version = f.readline().split("version")[1].strip()
except FileNotFoundError:
    print("Skipping timezone database generation. No timezone information available")
    sys.exit(0)

if generated_db_version >= installed_db_version:
    print("Skipping timezone database generation. Installed version {} is not newer than last generated version {}".format(installed_db_version, generated_db_version))
    sys.exit(0)

timezones = OrderedDict(sorted(make_timezones_dict().items(), key=lambda x: x[0]))

nested_dict = OrderedDict()
empty_nested_dict = OrderedDict()

for name, tz in timezones.items():
    splt = name.split("/")
    d = nested_dict
    d2 = empty_nested_dict
    for entry in splt[:-1]:
        d = d.setdefault(entry, OrderedDict())
        d2 = d2.setdefault(entry, OrderedDict())

    d[splt[-1]] = tz
    d2[splt[-1]] = None

generate("global", nested_dict)

util.specialize_template("timezones.c.template", "timezones.c", {
    '{{{generated_comment}}}': "/*\n{};{}\n*/".format(installed_db_version, datetime.datetime.utcnow().isoformat()),
    '{{{table_inits}}}': "\n\n".join(inits)
})


util.specialize_template("../../../web/src/modules/ntp/timezones.ts.template", "../../../web/src/modules/ntp/timezones.ts", {
    '{{{generated_comment}}}': "/*\n{};{}\n*/".format(installed_db_version, datetime.datetime.utcnow().isoformat()),
    '{{{json}}}': json.dumps(empty_nested_dict, indent=4)
})
