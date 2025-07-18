import os
import json
import sys
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software import util

def flatten(list_of_lists):
    return sum(list_of_lists, [])

def get_and_delete(d, keys):
    last_d = None
    for k in keys:
        last_d = d
        d = d[k]
    del last_d[k]
    return d

def get_all_ts_files(folder):
    result = []
    for root, dirs, files in os.walk(folder):
        for name in files:
            if not name.endswith(".ts") and not name.endswith(".tsx"):
                continue
            result.append(os.path.join(root, name))
    return result

def main():
    ts_files = [os.path.join("src_tfpp", "main.tsx"), os.path.join("src_tfpp", "app.tsx")]

    ts_files += get_all_ts_files("./src_tfpp/ts")
    ts_files += get_all_ts_files("./src_tfpp/typings")

    for frontend_module in sys.argv[1:]:
        ts_files += get_all_ts_files(os.path.join("src_tfpp", "modules", frontend_module))

    used_placeholders, template_literals = util.parse_ts_files(ts_files)

    with open('./src/ts/translation.json', 'r', encoding='utf-8') as f:
        translation = json.loads(f.read())

    used_placeholders = set(used_placeholders)
    used_but_missing = []

    for p in used_placeholders:
        keys = p.split(".")
        for l, v in translation.items():
            try:
                get_and_delete(v, keys)
            except KeyError:
                used_but_missing.append(l + '.' + p)

    if len(used_but_missing):
        print(util.red("Missing placeholders:"))
        for x in sorted(used_but_missing):
            print("\t" + util.red(x))

    unused = util.get_nested_keys(translation)
    for k, v in template_literals.items():
        prefix = k.split('${')[0]
        suffix = k.split('}')[1]
        to_remove = []
        for x in unused:
            lang, rest = x.split(".", 1)
            if rest.startswith(prefix) and rest.endswith(suffix):
                replacement = rest.replace(prefix, "").replace(suffix, "")
                v.append((lang, replacement))
                to_remove.append(x)

        unused = [x for x in unused if not x in to_remove]

    if len(unused) > 0:
        unused_filtered = []
        invisible = [
            '.debug.content.spi0',
            '.debug.content.spi1',
            '.debug.content.spi2',
            '.debug.content.spi3',
            '.em_debug.content.debug_description',
            '.em_debug.content.debug_description_muted',
            '.em_debug.content.debug_start',
            '.em_debug.content.debug_stop',
            '.em_debug.script.debug_done',
            '.em_debug.script.debug_running',
            '.em_debug.script.debug_stop_failed',
            '.em_debug.script.debug_stopped',
            '.em_debug.script.loading_debug_report',
            '.em_debug.script.loading_debug_report_failed',
            '.em_debug.script.loading_event_log',
            '.em_debug.script.loading_event_log_failed',
            '.em_debug.script.starting_debug',
            '.em_debug.script.starting_debug_failed',
            '.em_debug.script.tab_close_warning',
            '.evse.content.debug_description',
            '.evse.content.debug_description_muted',
            '.evse.content.debug_start',
            '.evse.content.debug_stop',
            '.evse.script.debug_done',
            '.evse.script.debug_running',
            '.evse.script.debug_stop_failed',
            '.evse.script.debug_stopped',
            '.evse.script.loading_debug_report',
            '.evse.script.loading_debug_report_failed',
            '.evse.script.loading_event_log',
            '.evse.script.loading_event_log_failed',
            '.evse.script.starting_debug',
            '.evse.script.starting_debug_failed',
            '.evse.script.tab_close_warning',
            '.firmware_update.script.downgrade',
            '.firmware_update.script.info_page_corrupted',
            '.firmware_update.script.no_info_page',
            '.firmware_update.script.wrong_firmware_type',
            '.evse.content.evse_v2_gpio_names_0',
            '.evse.content.evse_v2_gpio_names_1',
            '.evse.content.evse_v2_gpio_names_2',
            '.evse.content.evse_v2_gpio_names_3',
            '.evse.content.evse_v2_gpio_names_4',
            '.evse.content.evse_v2_gpio_names_5',
            '.evse.content.evse_v3_gpio_names_0',
            '.evse.content.evse_v3_gpio_names_1',
            '.evse.content.evse_v3_gpio_names_2',
            '.evse.content.evse_v3_gpio_names_3',
            '.evse.content.evse_v3_gpio_names_4',
            '.evse.content.evse_v3_gpio_names_5',
            '.firmware_update.script.install_state_2',
            '.firmware_update.script.install_state_3',
            '.firmware_update.script.install_state_4',
            '.firmware_update.script.install_state_5',
            '.firmware_update.script.install_state_6',
            '.firmware_update.script.install_state_7',
            '.firmware_update.script.install_state_8',
            '.firmware_update.script.install_state_9',
            '.firmware_update.script.install_state_10',
            '.firmware_update.script.install_state_11',
            '.firmware_update.script.install_state_12',
            '.firmware_update.script.install_state_13',
            '.firmware_update.script.install_state_14',
            '.firmware_update.script.install_state_15',
            '.firmware_update.script.install_state_16',
            '.firmware_update.script.install_state_17',
            '.firmware_update.script.install_state_18',
            '.firmware_update.script.install_state_19',
            '.firmware_update.script.install_state_24',
            '.firmware_update.script.install_state_20',
            '.firmware_update.script.install_state_21',
            '.firmware_update.script.install_state_23',
            '.firmware_update.script.install_state_25',
            '.firmware_update.script.install_state_26',
            '.firmware_update.script.install_state_27',
        ]

        for x in sorted(unused):
            for y in invisible:
                if x.endswith(y):
                    break
            else:
                unused_filtered.append(x)

        print("Unused placeholders:")
        for x in sorted(unused_filtered):
            print("\t" + x)

if __name__ == "__main__":
    main()
