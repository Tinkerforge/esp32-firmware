import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

plane_count = 6

with open('api.ts', 'w', encoding='utf-8') as f:
    f.write(tfutil.specialize_template('api.ts.template_header', None, {}) + '\n')

    for i in range(plane_count):
        f.write(tfutil.specialize_template('api.ts.template_fragment', None, {
            "{{{plane_id}}}": str(i),
        }) + '\n')
