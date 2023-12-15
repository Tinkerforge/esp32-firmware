import os
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
import base64

dir = sys.argv[1]

with open(dir + '/favicon_192.png', 'rb') as file:
    icon_small = base64.b64encode(file.read()).decode('utf-8')
with open(dir + '/favicon_512.png', 'rb') as file:
    icon_big = base64.b64encode(file.read()).decode('utf-8')
with open(dir + '/pre.scss', 'r', encoding='utf-8') as file:
    color = file.read().split('\n')[1].split(' ')[1]
    if color.endswith(';'):
        color = color[:-1]

with open('manifest.json.template', 'r', encoding='utf-8') as f:
    content = f.read()

content = content.replace(" ", "").replace("\n", "").replace("maskableany", "maskable any")

with open('manifest.json', 'w', encoding='utf-8') as f:
    f.write(content)


util.specialize_template('manifest.json', 'manifest.json', {
    '{{{theme_color}}}': color,
    '{{{small_icon}}}': 'data:image/png;base64, ' + icon_small,
    '{{{big_icon}}}': 'data:image/png;base64, ' +  icon_big
})

with open('manifest.json', 'r', encoding='utf-8') as file:
    util.embed_data_with_digest(file.read().encode('utf-8'), '.', 'manifest', 'char')
