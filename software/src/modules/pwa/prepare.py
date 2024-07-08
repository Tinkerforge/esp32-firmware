import sys
import base64
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util

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


tfutil.specialize_template('manifest.json', 'manifest.json', {
    '{{{theme_color}}}': color,
    '{{{small_icon}}}': 'data:image/png;base64, ' + icon_small,
    '{{{big_icon}}}': 'data:image/png;base64, ' +  icon_big
})

with open('manifest.json', 'r', encoding='utf-8') as file:
    util.embed_data_with_digest(file.read().encode('utf-8'), '.', 'manifest', 'char')
