import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util

with open('recovery.html', 'rb') as f:
    util.embed_data_with_digest(f.read(), '.', 'recovery_html', 'char', data_filter=util.gzip_compress)
