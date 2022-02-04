import os
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

new_digest = h.hexdigest()

try:
    with open('login.html.h.digest', 'r') as f:
        old_digest = f.read().strip()
except FileNotFoundError:
    old_digest = None

if old_digest != new_digest or not os.path.exists('login.html.h'):
    try:
        os.remove('login.html.h')
    except FileNotFoundError:
        pass

    try:
        os.remove('login.html.h.digest')
    except FileNotFoundError:
        pass

    with ChangedDirectory('login_page_ignored'):
        if not os.path.isdir("node_modules"):
            print("Authentication web interface dependencies not installed. Installing now.")
            subprocess.run(["npm", "ci"])

        subprocess.run(["npx", "gulp"])

    shutil.copy2("login_page_ignored/dist/login.html.h", "login.html.h")

    with open('login.html.h.digest', 'w') as f:
        f.write(new_digest)
