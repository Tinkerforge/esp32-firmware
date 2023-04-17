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
cur_path =  os.path.abspath(os.getcwd())

with open(dir + '/favicon_192.png', 'rb') as file:
    icon_small = base64.b64encode(file.read()).decode('utf-8')
with open(dir + '/favicon_512.png', 'rb') as file:
    icon_big = base64.b64encode(file.read()).decode('utf-8')
with open(dir + "/pre.scss") as file:
    color = file.read().split('\n')[1].split(' ')[1].removesuffix(';')

template_path = dir[0:dir.find('src')]

util.specialize_template(os.path.join(template_path, 'manifest.json.template'), os.path.join(cur_path, 'manifest.json'), {
    '{{{theme_color}}}': color,
    '{{{small_icon}}}': 'data:image/png;base64, ' + icon_small,
    '{{{big_icon}}}': 'data:image/png;base64, ' +  icon_big
})

with open(os.path.join(cur_path, 'manifest.json')) as file:
    util.embed_data_with_digest(file.read().encode('utf-8'), cur_path, 'manifest', 'char')