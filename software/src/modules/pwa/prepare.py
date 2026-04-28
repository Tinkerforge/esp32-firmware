import sys
import os
import base64
import json
import pathlib
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util

branding_mod_path = pathlib.Path(util.get_env_metadata()['branding_mod_path'])

with open(branding_mod_path / 'favicon_192.png', 'rb') as f:
    icon_small = base64.b64encode(f.read()).decode('utf-8')

with open(branding_mod_path / 'favicon_512.png', 'rb') as f:
    icon_big = base64.b64encode(f.read()).decode('utf-8')

with open(branding_mod_path / 'pre.scss', 'r', encoding='utf-8') as f:
    color = f.read().split('\n')[1].split(' ')[1]
    if color.endswith(';'):
        color = color[:-1]

content = tfutil.specialize_template('manifest.json.template', None, {
    '{{{theme_color}}}': color,
    '{{{small_icon}}}': icon_small,
    '{{{big_icon}}}': icon_big,
}).replace(" ", "").replace("\n", "").replace("maskableany", "maskable any").strip()

util.embed_data_with_digest(content.encode('utf-8'), 'generated', 'manifest', 'char', 'ssize_t')
