import os
import shutil
import subprocess

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

# Generate web interface
with ChangedDirectory('login_page_ignored'):
    if not os.path.isdir("node_modules"):
        print("Authentication web interface dependencies not installed. Installing now.")
        subprocess.run(["npm", "install", "--save-dev"])
    subprocess.run(["npx", "gulp"])

shutil.copy2("login_page_ignored/dist/login.html.h", "./login.html.h")
