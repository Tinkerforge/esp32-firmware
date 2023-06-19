import json
import os
import re
import sys

import importlib.util
import importlib.machinery

software_dir = os.path.realpath(os.path.join(os.path.dirname(__file__), '..'))

def create_software_module():
    software_spec = importlib.util.spec_from_file_location('software', os.path.join(software_dir, '__init__.py'))
    software_module = importlib.util.module_from_spec(software_spec)

    software_spec.loader.exec_module(software_module)

    sys.modules['software'] = software_module

if 'software' not in sys.modules:
    create_software_module()

from software import util

def flatten(list_of_lists):
    return sum(list_of_lists, [])

def get_and_delete(d, keys):
    last_d = None
    for k in keys:
        last_d = d
        d = d[k]
    del last_d[k]
    return d

def get_all_ts_files(folder):
    result = []
    for root, dirs, files in os.walk(folder):
        for name in files:
            if not name.endswith(".ts") and not name.endswith(".tsx"):
                continue
            result.append(os.path.join(root, name))
    return result

def check_mismatch(translation_values, key, p):
    last = None
    mismatches = []

    for translation_value in translation_values:
        if isinstance(translation_value[key], dict):
            for subkey in translation_value[key]:
                mismatches += check_mismatch([translation_value[key] for translation_value in translation_values], subkey, p + key + '.')
        elif last == None:
            last = translation_value[key]
        else:
            a = last[:1]
            b = translation_value[key][:1]

            if a.isupper() != b.isupper() or a.isalpha() != b.isalpha():
                mismatches.append((p + key, repr(last), repr(translation_value[key])))

    return mismatches

def main():
    ts_files = [os.path.join("src", "main.ts")]

    ts_files += get_all_ts_files("./src/ts")
    ts_files += get_all_ts_files("./src/typings")

    for frontend_module in sys.argv[1:]:
        ts_files += get_all_ts_files(os.path.join("src", "modules", frontend_module))

    used_placeholders, template_literals = util.parse_ts_files(ts_files)

    with open('./src/ts/translation.json', 'r', encoding='utf-8') as f:
        translation = json.loads(f.read())

    mismatches = []

    for key in translation['en']:
        mismatches += check_mismatch(translation.values(), key, '')

    if len(mismatches):
        print("Mismatches:")
        reported_mismatches = []
        for x in sorted(mismatches):
            if x not in reported_mismatches:
                print("\t" + x[0] + " " + x[1] + " " + x[2])
                reported_mismatches.append(x)

    with open('./src/index.html', 'r', encoding='utf-8') as f:
        content = f.read()

    used_placeholders += flatten([x.split(" ") for x in re.findall('data-i18n="([^"]*)"', content)])
    used_placeholders = set(used_placeholders)
    used_but_missing = []

    for p in used_placeholders:
        keys = p.split(".")
        for l, v in translation.items():
            try:
                get_and_delete(v, keys)
            except KeyError:
                used_but_missing.append(l + '.' + p)

    if len(used_but_missing):
        print(util.red("Missing placeholders:"))
        for x in sorted(used_but_missing):
            print("\t" + util.red(x))

    unused = util.get_nested_keys(translation)
    for k, v in template_literals.items():
        prefix = k.split('${')[0]
        suffix = k.split('}')[1]
        to_remove = []
        for x in unused:
            lang, rest = x.split(".", 1)
            if rest.startswith(prefix) and rest.endswith(suffix):
                replacement = rest.replace(prefix, "").replace(suffix, "")
                v.append((lang, replacement))
                to_remove.append(x)

        unused = [x for x in unused if not x in to_remove]

    if len(unused) > 0:
        print("Unused placeholders:")
        for x in sorted(unused):
            print("\t" + x)

if __name__ == "__main__":
    main()
