import os
import shutil

def recreate_dir(path):
    if os.path.exists(path):
        shutil.rmtree(path)

    os.makedirs(path)

def specialize_template(template_filename, destination_filename, replacements, check_completeness=True, remove_template=False):
    lines = []
    replaced = set()

    with open(template_filename, 'r') as f:
        for line in f.readlines():
            for key in replacements:
                replaced_line = line.replace(key, replacements[key])

                if replaced_line != line:
                    replaced.add(key)

                line = replaced_line

            lines.append(line)

    if check_completeness and replaced != set(replacements.keys()):
        raise Exception('Not all replacements for {0} have been applied. Missing are {1}'.format(template_filename, ', '.join(set(replacements.keys() - replaced))))

    with open(destination_filename, 'w') as f:
        f.writelines(lines)

    if remove_template:
        os.remove(template_filename)

recreate_dir('dist')

with open('assets/css/login.css') as f:
    login_css = f.read()

with open('assets/js/login_bundle.js') as f:
    login_js = f.read()

specialize_template('assets/login.html', 'dist/login.html',
    {
        '<link href=css/login.css rel=stylesheet>': '<style rel=stylesheet>{}</style>'.format(login_css),
        '<script src=js/login_bundle.js></script>': '<script>{}</script>'.format(login_js)
    })
