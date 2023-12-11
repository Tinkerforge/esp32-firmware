import os
import re
import sys

html_id = re.compile('id=["\']([^"\'\s]*)["\'\s]')
jquery_id = re.compile('\$\([\'"]#([^\'"\s,]*)[\'"\s]') #todo handle more complex css selectors
js_id = re.compile('getElementById\([\'"]([^\'"\s]*)[\'"\s]')

with open('./src/index.html', 'r', encoding='utf-8') as f:
    html_ids = set(html_id.findall(f.read()))

ts_files = []
for root, dirs, files in os.walk("./src/ts"):
    for name in files:
        if not name.endswith(".ts"):
            continue
        ts_files.append(os.path.join(root, name))

for frontend_module in sys.argv[1:]:
    folder = os.path.join("src", "modules", frontend_module)

    if os.path.exists(os.path.join(folder, "main.ts")):
        ts_files.append(os.path.join(folder, "main.ts"))

    if os.path.exists(os.path.join(folder, "translation_de.ts")):
        ts_files.append(os.path.join(folder, "translation_de.ts"))

    if os.path.exists(os.path.join(folder, "translation_en.ts")):
        ts_files.append(os.path.join(folder, "translation_en.ts"))

ts_files.append(os.path.join("src", "main.tsx"))

js_ids = set()
for file in ts_files:
    with open(file, 'r', encoding='utf-8') as f:
        content = f.read()
    js_ids.update(jquery_id.findall(content))
    js_ids.update(js_id.findall(content))
    html_ids.update(html_id.findall(content))

missing = sorted(list(js_ids - html_ids))
for x in missing:
    print("ID {} is used in Typescript but can not be found in HTML".format(x))
