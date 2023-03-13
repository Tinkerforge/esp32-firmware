import json
import os
import re
import sys

import configparser

import importlib.util
import importlib.machinery

software_dir = os.path.realpath(os.path.join(os.path.dirname(__file__), '..'))

def create_software_module():
    software_spec = importlib.util.spec_from_file_location('software', os.path.join(software_dir, '__init__.py'))
    software_module = importlib.util.module_from_spec(software_spec)

    software_spec.loader.exec_module(software_module)

    sys.modules['software'] = software_module

if 'software' not in sys.modules:
    create_software_module()

from software import util

def main():
    config = configparser.ConfigParser()
    config.read(os.path.join(software_dir, "platformio.ini"))

    envs = {}

    for section in config.sections():
        if "frontend_modules" in config[section].keys():
            envs[section] = [x.lower().replace(" ", "_") for x in config[section]["frontend_modules"].splitlines()]

    env_overrides = {}

    langs = set()
    modules = set()

    for env, mods in envs.items():
        override_files = []
        for mod in mods:
            folder = os.path.join("src", "modules", mod)

            if os.path.exists(os.path.join(folder, "translation_de_override.json")):
                override_files.append(os.path.join(folder, "translation_de_override.json"))

            if os.path.exists(os.path.join(folder, "translation_en_override.json")):
                override_files.append(os.path.join(folder, "translation_en_override.json"))

        translation = {}
        for f in override_files:
            with open(f, 'r', encoding='utf-8') as file:
                util.merge(translation, {re.search("translation_([^_]*)_override", f).groups()[0]: json.load(file)})

        env_overrides[env] = {}
        for lang, v in translation.items():
            env_overrides[env][lang] = {}
            langs.add(lang)
            for mod, replacements in v.items():
                modules.add(mod)
                env_overrides[env][lang][mod] = util.get_nested_keys(replacements)

    for lang in langs:
        for mod in modules:
            envs_with_mod = {env: v for env, v in env_overrides.items() if mod in v[lang]}

            if len(envs_with_mod) == 0:
                continue

            keys = list(envs_with_mod.keys())
            first_env = set(envs_with_mod[keys[0]][lang][mod])

            if not all(first_env == set(envs_with_mod[x][lang][mod]) for x in keys[1:]):
                print(util.red("Overrides out of sync for {}.{}".format(lang, mod)))


if __name__ == "__main__":
    main()
