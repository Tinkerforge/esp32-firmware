import os
import json
import re
import configparser
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util

def main():
    config = configparser.ConfigParser()
    config.read('../platformio.ini')

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
