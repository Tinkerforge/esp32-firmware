import json
import os
import re
import sys
import os

START_PATTERN = "{[index: string]:any} = "
END_PATTERN = "};"

def merge(left, right, path=[]):
    for key in right:
        if key in left:
            if isinstance(left[key], dict) and isinstance(right[key], dict):
                merge(left[key], right[key], path + [str(key)])
            else:
                print("Found ambiguous placeholder ", '.'.join(path + [key]))
        else:
            left[key] = right[key]
    return left

def flatten(list_of_lists):
    return sum(list_of_lists, [])

def get_and_delete(d, keys):
    last_d = None
    for k in keys:
        last_d = d
        d = d[k]
    del last_d[k]
    return d

def get_nested_keys(d, path=""):
    r = []
    for k, v in d.items():
        if isinstance(v, dict) and len(v) > 0:
            r += get_nested_keys(v, path + "." + k if path != "" else k)
        elif isinstance(v, str):
            r.append(path + "." + k)
    return r


translation = {}
used_placeholders = []
template_literals = {}

def parse_ts_file(path, name):
    global translation
    global used_placeholders
    global template_literals

    with open(path) as f:
        content = f.read()

    placeholders = re.findall('__\(([^\)]*)', content)
    try:
        placeholders.remove("s: string")
    except:
        pass

    template_literal_keys = [x for x in placeholders if x[0] == '`' and x[-1] == '`' and '${' in x and '}' in x]
    placeholders = [x for x in placeholders if x not in template_literal_keys]

    template_literals.update({x[1:-1]: [] for x in template_literal_keys})

    incorrect_placeholders = [x for x in placeholders if not x[0] == '"' or not x[-1] == '"']
    if len(incorrect_placeholders) != 0:
        print("Found incorrectly quoted placeholders. Use \"\"!", incorrect_placeholders)

    used_placeholders += [x[1:-1] for x in placeholders]

    if not name.startswith("translation_") or not name.endswith(".ts"):
        return

    language = name[len('translation_'):-len('.ts')]
    start = content.find(START_PATTERN)
    while start >= 0:
        content = content[start+len(START_PATTERN):]
        end = content.find(END_PATTERN)
        json_dict = content[:end+1]
        json_dict = re.sub(",\s*\}", "}", json_dict)
        for x in re.findall('"([^"]*)":\s*""', json_dict):
            print('error: key "{}" in {} has empty value. Use {{{{{{empty_text}}}}}} instead.'.format(x, name))
        json_dict = json_dict.replace("{{{empty_text}}}", '""')
        try:
            merge(translation, {language: json.loads(json_dict)})
        except Exception as e:
            print("error:", e, json_dict)

        content = content[end+len(END_PATTERN):]
        start = content.find(START_PATTERN)

def main():
    global translation
    global used_placeholders
    global template_literals

    for root, dirs, files in os.walk("./src/ts"):
        for name in files:
            if not name.endswith(".ts"):
                continue
            parse_ts_file(os.path.join(root, name), name)

    for root, dirs, files in os.walk("./src/typings"):
        for name in files:
            if not name.endswith(".ts"):
                continue
            parse_ts_file(os.path.join(root, name), name)
    parse_ts_file(os.path.join("src", "main.ts"), "main.ts")

    for frontend_module in sys.argv[1:]:
        folder = os.path.join("src", "modules", frontend_module)

        if os.path.exists(os.path.join(folder, "main.ts")):
            parse_ts_file(os.path.join(folder, "main.ts"), "main.ts")

        if os.path.exists(os.path.join(folder, "translation_de.ts")):
            parse_ts_file(os.path.join(folder, "translation_de.ts"), "translation_de.ts")

        if os.path.exists(os.path.join(folder, "translation_en.ts")):
            parse_ts_file(os.path.join(folder, "translation_en.ts"), "translation_en.ts")

    assert len(translation) > 0

    with open("./src/index.html") as f:
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
        print("Missing placeholders:")
        for x in sorted(used_but_missing):
            print("\t" + x)

    unused = get_nested_keys(translation)
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
