import os
import sys
import shutil
import subprocess
import hashlib

# use "with ChangedDirectory('/path/to/abc')" instead of "os.chdir('/path/to/abc')"
class ChangedDirectory(object):
    def __init__(self, path):
        self.path = path
        self.previous_path = None

    def __enter__(self):
        self.previous_path = os.getcwd()
        os.chdir(self.path)

    def __exit__(self, type_, value, traceback):
        os.chdir(self.previous_path)

# ensure node_modules is up-to-date
with ChangedDirectory('login_page_ignored'):
    with open('package-lock.json', 'rb') as f:
        new_node_digest = hashlib.sha256(f.read()).hexdigest()

    try:
        with open('package-lock.json.digest', 'r', encoding='utf-8') as f:
            old_node_digest = f.read().strip()
    except FileNotFoundError:
        old_node_digest = None

    if old_node_digest != new_node_digest or not os.path.exists('node_modules/.complete'):
        print("Login page web interface dependencies not up-to-date. Updating now.")

        try:
            os.remove('package-lock.json.digest')
        except FileNotFoundError:
            pass

        try:
            shutil.rmtree('node_modules')
        except FileNotFoundError:
            pass

        subprocess.check_call(["npm", "ci"], shell=sys.platform == 'win32')

        with open('node_modules/.complete', 'wb') as f:
            pass

        with open('package-lock.json.digest', 'w', encoding='utf-8') as f:
            f.write(new_node_digest)

# build web interface
h = hashlib.sha256()

for name in sorted(os.listdir('login_page_ignored')):
    path = os.path.join('login_page_ignored', name)

    if not os.path.isfile(path):
        continue

    with open(path, 'rb') as f:
        h.update(f.read())

for root, dirs, files in sorted(os.walk('login_page_ignored/src')):
    for name in files:
        path = os.path.join(root, name)

        with open(path, 'rb') as f:
            h.update(f.read())

new_html_digest = h.hexdigest()

try:
    with open('login.html.h.digest', 'r') as f:
        old_html_digest = f.read().strip()
except FileNotFoundError:
    old_html_digest = None

if old_html_digest != new_html_digest or not os.path.exists('login.html.h'):
    try:
        os.remove('login.html.h.digest')
    except FileNotFoundError:
        pass

    try:
        os.remove('login.html.h')
    except FileNotFoundError:
        pass

    with ChangedDirectory('login_page_ignored'):
        try:
            shutil.rmtree('dist')
        except FileNotFoundError:
            pass

        environ = dict(os.environ)
        environ['PYTHON_EXECUTABLE'] = sys.executable
        subprocess.check_call(["npx", "gulp"], env=environ, shell=sys.platform == 'win32')

    shutil.copy2("login_page_ignored/dist/login.html.h", "login.html.h")

    with open('login.html.h.digest', 'w') as f:
        f.write(new_html_digest)
