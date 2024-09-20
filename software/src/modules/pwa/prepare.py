import sys
import os
import base64
import json
import pathlib
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util

metadata_json = os.getenv('PLATFORMIO_METADATA')

if metadata_json == None:
    print('$PLATFORMIO_METADATA not set')
    sys.exit(-1)

metadata = json.loads(metadata_json)
branding_mod_path = pathlib.Path(metadata['branding_mod_path'])

with open(branding_mod_path / 'favicon_192.png', 'rb') as f:
    icon_small = base64.b64encode(f.read()).decode('utf-8')

with open(branding_mod_path / 'favicon_512.png', 'rb') as f:
    icon_big = base64.b64encode(f.read()).decode('utf-8')

with open(branding_mod_path / 'pre.scss', 'r', encoding='utf-8') as f:
    color = f.read().split('\n')[1].split(' ')[1]
    if color.endswith(';'):
        color = color[:-1]

with open('manifest.json.template', 'r', encoding='utf-8') as f:
    content = f.read()

content = content.replace(" ", "").replace("\n", "").replace("maskableany", "maskable any")

with open('manifest.json', 'w', encoding='utf-8') as f:
    f.write(content)

tfutil.specialize_template('manifest.json', 'manifest.json', {
    '{{{theme_color}}}': color,
    '{{{small_icon}}}': 'data:image/png;base64, ' + icon_small,
    '{{{big_icon}}}': 'data:image/png;base64, ' + icon_big,
})

with open('manifest.json', 'r', encoding='utf-8') as f:
    util.embed_data_with_digest(f.read().encode('utf-8'), '.', 'manifest', 'char')
